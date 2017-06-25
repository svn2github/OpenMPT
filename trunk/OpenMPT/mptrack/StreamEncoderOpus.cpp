/*
 * StreamEncoderOpus.cpp
 * ---------------------
 * Purpose: Exporting streamed music files.
 * Notes  : none
 * Authors: Joern Heusipp
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#include "stdafx.h"

#include "StreamEncoder.h"
#include "StreamEncoderOpus.h"

#include "../common/mptIO.h"
#include "../common/mptBufferIO.h"

#include "../common/ComponentManager.h"

#include "Mptrack.h"

#include <deque>

#if defined(MPT_WITH_OPUS) && defined(MPT_WITH_OPUSENC)
#include <opusenc.h>
#endif


OPENMPT_NAMESPACE_BEGIN



static Encoder::Traits BuildTraits()
	{
		Encoder::Traits traits;
#if defined(MPT_WITH_OPUS) && defined(MPT_WITH_OPUSENC)
		traits.fileExtension = MPT_PATHSTRING("opus");
		traits.fileShortDescription = MPT_USTRING("Opus");
		traits.encoderSettingsName = MPT_USTRING("Opus");
		traits.canTags = true;
		traits.maxChannels = 4;
		traits.samplerates = mpt::make_vector(opus_all_samplerates);
		traits.modes = Encoder::ModeCBR | Encoder::ModeVBR;
		traits.bitrates = mpt::make_vector(opus_bitrates);
		traits.defaultSamplerate = 48000;
		traits.defaultChannels = 2;
		traits.defaultMode = Encoder::ModeVBR;
		traits.defaultBitrate = 128;
#endif
		return traits;
	}



#if defined(MPT_WITH_OPUS) && defined(MPT_WITH_OPUSENC)

class OpusStreamWriter : public StreamWriterBase
{
private:
	OpusEncCallbacks ope_callbacks;
	OggOpusComments *ope_comments;
	OggOpusEnc *ope_encoder;
	std::vector<std::pair<std::string, std::string> > opus_comments;
private:
	static int CallbackWrite(void *user_data, const unsigned char *ptr, opus_int32 len)
	{
		return reinterpret_cast<OpusStreamWriter*>(user_data)->CallbackWriteImpl(ptr, len);
	}
	static int CallbackClose(void *user_data)
	{
		return reinterpret_cast<OpusStreamWriter*>(user_data)->CallbackCloseImpl();
	}
	int CallbackWriteImpl(const unsigned char *ptr, opus_int32 len)
	{
		if(len < 0)
		{
			return 0;
		}
		if(!ptr)
		{
			return 0;
		}
		buf.assign(ptr, ptr + len);
		WriteBuffer();
		return len;
	}
	int CallbackCloseImpl()
	{
		return 0;
	}
private:
	void AddCommentField(const std::string &field, const mpt::ustring &data)
	{
		if(!field.empty() && !data.empty())
		{
			opus_comments.push_back(std::make_pair(field, mpt::ToCharset(mpt::CharsetUTF8, data)));
		}
	}
public:
	OpusStreamWriter(std::ostream &stream, const Encoder::Settings &settings, const FileTags &tags)
		: StreamWriterBase(stream)
	{
		ope_callbacks.write = &CallbackWrite;
		ope_callbacks.close = &CallbackClose;
		opus_comments.clear();

		bool opus_cbr = (settings.Mode == Encoder::ModeCBR);
		int opus_bitrate = settings.Bitrate * 1000;

		if(settings.Tags)
		{
			AddCommentField("ENCODER",     tags.encoder);
			AddCommentField("SOURCEMEDIA", MPT_USTRING("tracked music file"));
			AddCommentField("TITLE",       tags.title          );
			AddCommentField("ARTIST",      tags.artist         );
			AddCommentField("ALBUM",       tags.album          );
			AddCommentField("DATE",        tags.year           );
			AddCommentField("COMMENT",     tags.comments       );
			AddCommentField("GENRE",       tags.genre          );
			AddCommentField("CONTACT",     tags.url            );
			AddCommentField("BPM",         tags.bpm            ); // non-standard
			AddCommentField("TRACKNUMBER", tags.trackno        );
		}

		int ope_error = 0;

		ope_comments = ope_comments_create();
		if(settings.Tags && ope_comments)
		{
			for(const auto & comment : opus_comments)
			{
				ope_comments_add(ope_comments, comment.first.c_str(), comment.second.c_str());
			}
		}

		ope_encoder = ope_encoder_create_callbacks(&ope_callbacks, this, ope_comments, settings.Samplerate, settings.Channels, settings.Channels > 2 ? 1 : 0, &ope_error);
		
		opus_int32 ctl_serial = mpt::random<uint32>(theApp.PRNG());
		ope_encoder_ctl(ope_encoder, OPE_SET_SERIALNO(ctl_serial));

		opus_int32 ctl_bitrate = opus_bitrate;
		ope_encoder_ctl(ope_encoder, OPUS_SET_BITRATE(ctl_bitrate));

		if(opus_cbr)
		{
			opus_int32 ctl_vbr = 0;
			ope_encoder_ctl(ope_encoder, OPUS_SET_VBR(ctl_vbr));
		} else
		{
			opus_int32 ctl_vbr = 1;
			ope_encoder_ctl(ope_encoder, OPUS_SET_VBR(ctl_vbr));
			opus_int32 ctl_vbrcontraint = 0;
			ope_encoder_ctl(ope_encoder, OPUS_SET_VBR_CONSTRAINT(ctl_vbrcontraint));
		}

		opus_int32 complexity = StreamEncoderSettings::Instance().OpusComplexity;
		if(complexity >= 0)
		{
			ope_encoder_ctl(ope_encoder, OPUS_SET_COMPLEXITY(complexity));
		}

		ope_encoder_flush_header(ope_encoder);
		
	}
	virtual void WriteInterleaved(size_t count, const float *interleaved)
	{
		ope_encoder_write_float(ope_encoder, interleaved, count);
	}
	virtual ~OpusStreamWriter()
	{
		ope_encoder_drain(ope_encoder);

		ope_encoder_destroy(ope_encoder);
		ope_encoder = NULL;

		ope_comments_destroy(ope_comments);
		ope_comments = NULL;
	}
};

#endif // MPT_WITH_OGG



OggOpusEncoder::OggOpusEncoder()
//------------------------------
{
	SetTraits(BuildTraits());
}


bool OggOpusEncoder::IsAvailable() const
//--------------------------------------
{
#if defined(MPT_WITH_OPUS) && defined(MPT_WITH_OPUSENC)
	return true;
#else
	return false;
#endif
}


OggOpusEncoder::~OggOpusEncoder()
//-------------------------------
{
	return;
}


std::unique_ptr<IAudioStreamEncoder> OggOpusEncoder::ConstructStreamEncoder(std::ostream &file, const Encoder::Settings &settings, const FileTags &tags) const
//------------------------------------------------------------------------------------------------------------------------------------------------------------
{
	if(!IsAvailable())
	{
		return nullptr;
	}
#if defined(MPT_WITH_OPUS) && defined(MPT_WITH_OPUSENC)
	return mpt::make_unique<OpusStreamWriter>(file, settings, tags);
#else
	return nullptr;
#endif
}



OPENMPT_NAMESPACE_END
