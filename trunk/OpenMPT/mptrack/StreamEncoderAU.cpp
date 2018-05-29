/*
 * StreamEncoderAU.cpp
 * -------------------
 * Purpose: Exporting streamed music files.
 * Notes  : none
 * Authors: Joern Heusipp
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#include "stdafx.h"

#include "StreamEncoder.h"
#include "StreamEncoderAU.h"

#include "Mptrack.h"
#include "TrackerSettings.h"

#include "../common/mptFileIO.h"


OPENMPT_NAMESPACE_BEGIN


class AUStreamWriter : public IAudioStreamEncoder
{
private:

	const AUEncoder &enc;
	std::ostream &f;
	Encoder::Format formatInfo;

private:
	static std::string TagToAnnotation(const std::string & field, const mpt::ustring & tag)
	{
		if(tag.empty())
		{
			return std::string();
		}
		return mpt::format("%1=%2\n")(field, mpt::ToCharset(mpt::CharsetUTF8, mpt::String::Replace(tag, MPT_USTRING("="), MPT_UTF8("\xEF\xBF\xBD")))); // U+FFFD
	}

public:
	AUStreamWriter(const AUEncoder &enc_, std::ostream &file, const Encoder::Settings &settings, const FileTags &tags)
		: enc(enc_)
		, f(file)
	{

		formatInfo = enc.GetTraits().formats[settings.Format];
		MPT_ASSERT(formatInfo.Sampleformat.IsValid());
		MPT_ASSERT(formatInfo.Samplerate > 0);
		MPT_ASSERT(formatInfo.Channels > 0);

		std::string annotation;
		std::size_t annotationSize = 0;
		std::size_t annotationTotalSize = 8;
		if(settings.Tags)
		{
			// same format as invented by sox and implemented by ffmpeg
			annotation += TagToAnnotation("title", tags.title);
			annotation += TagToAnnotation("artist", tags.artist);
			annotation += TagToAnnotation("album", tags.album);
			annotation += TagToAnnotation("track", tags.trackno);
			annotation += TagToAnnotation("genre", tags.genre);
			annotation += TagToAnnotation("comment", tags.comments);
			annotationSize = annotation.length() + 1;
			annotationTotalSize = annotationSize;
			if(StreamEncoderSettings::Instance().AUPaddingAlignHint > 0)
			{
				annotationTotalSize = Util::AlignUp<std::size_t>(24u + annotationTotalSize, StreamEncoderSettings::Instance().AUPaddingAlignHint) - 24u;
			}
			annotationTotalSize = Util::AlignUp<std::size_t>(annotationTotalSize, 8u);
		}
		MPT_ASSERT(annotationTotalSize >= annotationSize);
		MPT_ASSERT(annotationTotalSize % 8 == 0);

		mpt::IO::WriteText(f, ".snd");
		mpt::IO::WriteIntBE<uint32>(f, mpt::saturate_cast<uint32>(24u + annotationTotalSize));
		mpt::IO::WriteIntBE<uint32>(f, ~uint32(0));
		uint32 encoding = 0;
		if(formatInfo.Sampleformat.IsFloat())
		{
			switch(formatInfo.Sampleformat.GetBitsPerSample())
			{
			case 32: encoding = 6; break;
			case 64: encoding = 7; break;
			}
		} else
		{
			switch(formatInfo.Sampleformat.GetBitsPerSample())
			{
			case  8: encoding = 2; break;
			case 16: encoding = 3; break;
			case 24: encoding = 4; break;
			case 32: encoding = 5; break;
			}
		}
		mpt::IO::WriteIntBE<uint32>(f, encoding);
		mpt::IO::WriteIntBE<uint32>(f, settings.Samplerate);
		mpt::IO::WriteIntBE<uint32>(f, settings.Channels);
		if(annotationSize > 0)
		{
			mpt::IO::WriteText(f, annotation);
			mpt::IO::WriteIntBE<uint8>(f, '\0');
		}
		for(std::size_t i = 0; i < annotationTotalSize - annotationSize; ++i)
		{
			mpt::IO::WriteIntBE<uint8>(f, '\0');
		}

	}
	mpt::endian GetConvertedEndianness() const override
	{
		return mpt::endian::big;
	}
	void WriteInterleaved(size_t count, const float *interleaved) override
	{
		MPT_ASSERT(formatInfo.Sampleformat.IsFloat());
		MPT_MAYBE_CONSTANT_IF(mpt::endian_is_big())
		{
			WriteInterleavedConverted(count, reinterpret_cast<const char*>(interleaved));
		} else
		{
			std::vector<IEEE754binary32BE> frameData(formatInfo.Channels);
			for(std::size_t frame = 0; frame < count; ++frame)
			{
				for(int channel = 0; channel < formatInfo.Channels; ++channel)
				{
					frameData[channel] = IEEE754binary32BE(interleaved[channel]);
				}
				mpt::IO::WriteRaw(f, reinterpret_cast<const char*>(frameData.data()), formatInfo.Channels * (formatInfo.Sampleformat.GetBitsPerSample()/8));
				interleaved += formatInfo.Channels;
			}
		}
	}
	void WriteInterleavedConverted(size_t frameCount, const char *data) override
	{
		if(formatInfo.Sampleformat.GetBitsPerSample() == 8)
		{
			for(std::size_t frame = 0; frame < frameCount; ++frame)
			{
				for(int channel = 0; channel < formatInfo.Channels; ++channel)
				{
					int8 sample = static_cast<int8>(static_cast<uint8>(*data) - 0x80);
					mpt::IO::WriteIntBE<int8>(f, sample);
					data++;
				}
			}
		} else
		{
			mpt::IO::WriteRaw(f, data, frameCount * formatInfo.Channels * (formatInfo.Sampleformat.GetBitsPerSample()/8));
		}
	}
	void WriteCues(const std::vector<uint64> &cues) override
	{
		MPT_UNREFERENCED_PARAMETER(cues);
	}
	virtual ~AUStreamWriter()
	{
		return;
	}
};



AUEncoder::AUEncoder()
{
	Encoder::Traits traits;
	traits.fileExtension = MPT_PATHSTRING("au");
	traits.fileShortDescription = MPT_USTRING("AU");
	traits.encoderSettingsName = MPT_USTRING("AU");
	traits.canTags = true;
	traits.canCues = false;
	traits.maxChannels = 4;
	traits.samplerates = TrackerSettings::Instance().GetSampleRates();
	traits.modes = Encoder::ModeEnumerated;
	for(std::size_t i = 0; i < traits.samplerates.size(); ++i)
	{
		int samplerate = traits.samplerates[i];
		for(int channels = 1; channels <= traits.maxChannels; channels *= 2)
		{
			for(int bytes = 5; bytes >= 1; --bytes)
			{
				Encoder::Format format;
				format.Samplerate = samplerate;
				format.Channels = channels;
				if(bytes == 5)
				{
					format.Sampleformat = SampleFormatFloat32;
					format.Description = MPT_USTRING("Floating Point");
				} else
				{
					format.Sampleformat = (SampleFormat)(bytes * 8);
					format.Description = mpt::format(MPT_USTRING("%1 Bit"))(bytes * 8);
				}
				format.Bitrate = 0;
				traits.formats.push_back(format);
			}
		}
	}
	traits.defaultSamplerate = 48000;
	traits.defaultChannels = 2;
	traits.defaultMode = Encoder::ModeEnumerated;
	traits.defaultFormat = 0;
	SetTraits(traits);
}


bool AUEncoder::IsAvailable() const
{
	return true;
}


std::unique_ptr<IAudioStreamEncoder> AUEncoder::ConstructStreamEncoder(std::ostream &file, const Encoder::Settings &settings, const FileTags &tags) const
{
	if(!IsAvailable())
	{
		return nullptr;
	}
	return mpt::make_unique<AUStreamWriter>(*this, file, settings, tags);
}


OPENMPT_NAMESPACE_END
