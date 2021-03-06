// generated 1999/12/18 23:45:52 CST by temporal@temporal.
// using glademm V0.5.5
//
// DO NOT EDIT THIS FILE ! It was created using
// glade-- /home/temporal/stf/src/modplug/glade/configure/modplug.glade
// for gtk 1.2.5 and gtkmm 1.0.2
//
// Please modify the corresponding derived classes in src/window1.cc

#include "configwin.h"

CConfigureWindow_Glade::CConfigureWindow_Glade()
	: Gtk_Window(GTK_WINDOW_TOPLEVEL),
	frame9("Resolution"),
	frame10("Channels"),
	frame11("Sampling Rate"),
	label10("Quality"),
	fxoversamp("Oversampling"),
	fxnr("Noise Reduction"),
	fxvolramp("Volume Ramping"),
	fxfastinfo("Fast Playlist Info"),
	frame12("General"),
	fxreverbon("Enable"),
	label11("Depth"),
	label12("Delay"),
	table6(2, 2, false),
	frame13("Reverb"),
	fxbasson("Enable"),
	label13("Amount"),
	label14("Range"),
	table7(2, 2, false),
	frame14("Bass Boost"),
	fxsurroundon("Enable"),
	label15("Depth"),
	label16("Delay"),
	table8(2, 2, false),
	frame15("Surround"),
	fxfadeon("Enable"),
	label17("Length\n(ms)"),
	frame16("Fade on Stop (WARINING: SEE README)"),
	label18("Effects"),
	okbtn("OK"),
	applybtn("Apply"),
	cancelbtn("Cancel")
{
	init1();
	init2();
	init3();
	init4();
	init5();
}

void CConfigureWindow_Glade::init1()
{
   bit16 = new Gtk_RadioButton(glademm_get_RadioGroup("bit"), "16 bit");
   glademm_set_RadioGroup("bit", bit16->group());
   bit8 = new Gtk_RadioButton(glademm_get_RadioGroup("bit"), "8 bit");
   glademm_set_RadioGroup("bit", bit8->group());
   stereo = new Gtk_RadioButton(glademm_get_RadioGroup("channels"), "Stereo");
   glademm_set_RadioGroup("channels", stereo->group());
   mono = new Gtk_RadioButton(glademm_get_RadioGroup("channels"), "Mono");
   glademm_set_RadioGroup("channels", mono->group());
   samp44 = new Gtk_RadioButton(glademm_get_RadioGroup("samprate"), "44 kHz");
   glademm_set_RadioGroup("samprate", samp44->group());
   samp22 = new Gtk_RadioButton(glademm_get_RadioGroup("samprate"), "22 kHz");
   glademm_set_RadioGroup("samprate", samp22->group());
   samp11 = new Gtk_RadioButton(glademm_get_RadioGroup("samprate"), "11 kHz");
   glademm_set_RadioGroup("samprate", samp11->group());
   fxreverbdepth_adj = new Gtk_Adjustment(1, 0, 100, 0, 0, 0);
   fxreverbdepth = new Gtk_HScale(*fxreverbdepth_adj);
   fxreverbdelay_adj = new Gtk_Adjustment(100, 40, 200, 0, 0, 0);
   fxreverbdelay = new Gtk_HScale(*fxreverbdelay_adj);
   fxbassamount_adj = new Gtk_Adjustment(6, 0, 100, 0, 0, 0);
   fxbassamount = new Gtk_HScale(*fxbassamount_adj);
   fxbassrange_adj = new Gtk_Adjustment(14, 10, 100, 0, 0, 0);
   fxbassrange = new Gtk_HScale(*fxbassrange_adj);
   fxsurrounddepth_adj = new Gtk_Adjustment(12, 0, 100, 0, 0, 0);
   fxsurrounddepth = new Gtk_HScale(*fxsurrounddepth_adj);
   fxsurrounddelay_adj = new Gtk_Adjustment(20, 5, 40, 0, 0, 0);
   fxsurrounddelay = new Gtk_HScale(*fxsurrounddelay_adj);
   fxfadetime_adj = new Gtk_Adjustment(500, 0, 2000, 0, 0, 0);
   fxfadetime = new Gtk_HScale(*fxfadetime_adj);
   bit16->set_name("bit16");
   connect_to_method(bit16->toggled, (CConfigureWindow*)this, &CConfigureWindow::on_bit16_toggled);
   bit8->set_name("bit8");
   connect_to_method(bit8->toggled, (CConfigureWindow*)this, &CConfigureWindow::on_bit8_toggled);
   vbox13.set_name("vbox13");
   vbox13.pack_start(*bit16, false, false, 0);
   vbox13.pack_start(*bit8, false, false, 0);
   frame9.set_name("frame9");
   frame9.set_label_align(0, 0);
   frame9.set_shadow_type(GTK_SHADOW_ETCHED_IN);
   frame9.add(&vbox13);
   stereo->set_name("stereo");
   connect_to_method(stereo->toggled, (CConfigureWindow*)this, &CConfigureWindow::on_stereo_toggled);
   mono->set_name("mono");
   connect_to_method(mono->toggled, (CConfigureWindow*)this, &CConfigureWindow::on_mono_toggled);
   vbox14.set_name("vbox14");
   vbox14.pack_start(*stereo, false, false, 0);
   vbox14.pack_start(*mono, false, false, 0);
   frame10.set_name("frame10");
   frame10.set_label_align(0, 0);
   frame10.set_shadow_type(GTK_SHADOW_ETCHED_IN);
   frame10.add(&vbox14);
   hbox8.set_name("hbox8");
   hbox8.pack_start(frame9);
   hbox8.pack_start(frame10);
   samp44->set_name("samp44");
   connect_to_method(samp44->toggled, (CConfigureWindow*)this, &CConfigureWindow::on_samp44_toggled);
}

void CConfigureWindow_Glade::init2()
{
   samp22->set_name("samp22");
   connect_to_method(samp22->toggled, (CConfigureWindow*)this, &CConfigureWindow::on_samp22_toggled);
   samp11->set_name("samp11");
   connect_to_method(samp11->toggled, (CConfigureWindow*)this, &CConfigureWindow::on_samp11_toggled);
   vbox15.set_name("vbox15");
   vbox15.pack_start(*samp44, false, false, 0);
   vbox15.pack_start(*samp22, false, false, 0);
   vbox15.pack_start(*samp11, false, false, 0);
   frame11.set_name("frame11");
   frame11.set_label_align(0, 0);
   frame11.set_shadow_type(GTK_SHADOW_ETCHED_IN);
   frame11.add(&vbox15);
   vbox12.set_name("vbox12");
   vbox12.pack_start(hbox8);
   vbox12.pack_start(frame11);
   label10.set_name("label10");
   label10.set_alignment(0.5, 0.5);
   label10.set_padding(0, 0);
   fxoversamp.set_name("fxOversamp");
   fxoversamp.set_active(true);
   connect_to_method(fxoversamp.toggled, (CConfigureWindow*)this, &CConfigureWindow::on_fxOversamp_toggled);
   fxnr.set_name("fxNR");
   fxnr.set_active(true);
   connect_to_method(fxnr.toggled, (CConfigureWindow*)this, &CConfigureWindow::on_fxNR_toggled);
   vbox17.set_name("vbox17");
   vbox17.pack_start(fxoversamp, false, false, 0);
   vbox17.pack_start(fxnr, false, false, 0);
   fxvolramp.set_name("fxVolRamp");
   fxvolramp.set_active(true);
   connect_to_method(fxvolramp.toggled, (CConfigureWindow*)this, &CConfigureWindow::on_fxVolRamp_toggled);
   fxfastinfo.set_name("fxFastinfo");
   fxfastinfo.set_active(true);
   connect_to_method(fxfastinfo.toggled, (CConfigureWindow*)this, &CConfigureWindow::on_fxFastinfo_toggled);
   vbox18.set_name("vbox18");
   vbox18.pack_start(fxvolramp, false, false, 0);
   vbox18.pack_start(fxfastinfo, false, false, 0);
   hbox9.set_name("hbox9");
   hbox9.pack_start(vbox17);
   hbox9.pack_start(vbox18);
   frame12.set_name("frame12");
   frame12.set_label_align(0, 0);
   frame12.set_shadow_type(GTK_SHADOW_ETCHED_IN);
   frame12.add(&hbox9);
   fxreverbon.set_name("fxReverbOn");
   fxreverbon.set_active(true);
   connect_to_method(fxreverbon.toggled, (CConfigureWindow*)this, &CConfigureWindow::on_fxReverbOn_toggled);
   fxreverbdepth->set_name("fxReverbDepth");
   fxreverbdepth->set_digits(1);
   fxreverbdepth->set_draw_value(false);
   fxreverbdepth->set_value_pos(GTK_POS_TOP);
//   connect_to_method(fxreverbdepth->drag_motion, (CConfigureWindow*)this, &CConfigureWindow::on_fxReverbDepth_drag_motion);
   fxreverbdelay->set_name("fxReverbDelay");
   fxreverbdelay->set_digits(1);
   fxreverbdelay->set_draw_value(false);
   fxreverbdelay->set_value_pos(GTK_POS_TOP);
//   connect_to_method(fxreverbdelay->drag_motion, (CConfigureWindow*)this, &CConfigureWindow::on_fxReverbDelay_drag_motion);
   label11.set_name("label11");
}

void CConfigureWindow_Glade::init3()
{
   label11.set_alignment(0.5, 0.5);
   label11.set_padding(0, 0);
   label12.set_name("label12");
   label12.set_alignment(0.5, 0.5);
   label12.set_padding(0, 0);
   table6.set_name("table6");
   table6.attach(*fxreverbdepth, 1, 2, 0, 1, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0);
   table6.attach(*fxreverbdelay, 1, 2, 1, 2, GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0);
   table6.attach(label11, 0, 1, 0, 1, 0, 0, 0, 0);
   table6.attach(label12, 0, 1, 1, 2, 0, 0, 0, 0);
   vbox19.set_name("vbox19");
   vbox19.pack_start(table6);
   hbox10.set_name("hbox10");
   hbox10.pack_start(fxreverbon, false, false, 0);
   hbox10.pack_start(vbox19);
   frame13.set_name("frame13");
   frame13.set_label_align(0, 0);
   frame13.set_shadow_type(GTK_SHADOW_ETCHED_IN);
   frame13.add(&hbox10);
   fxbasson.set_name("fxBassOn");
   fxbasson.set_active(true);
   connect_to_method(fxbasson.toggled, (CConfigureWindow*)this, &CConfigureWindow::on_fxBassOn_toggled);
   fxbassamount->set_name("fxBassAmount");
   fxbassamount->set_digits(1);
   fxbassamount->set_draw_value(false);
   fxbassamount->set_value_pos(GTK_POS_TOP);
//   connect_to_method(fxbassamount->drag_motion, (CConfigureWindow*)this, &CConfigureWindow::on_fxBassAmount_drag_motion);
   fxbassrange->set_name("fxBassRange");
   fxbassrange->set_digits(1);
   fxbassrange->set_draw_value(false);
   fxbassrange->set_value_pos(GTK_POS_TOP);
//   connect_to_method(fxbassrange->drag_motion, (CConfigureWindow*)this, &CConfigureWindow::on_fxBassRange_drag_motion);
   label13.set_name("label13");
   label13.set_alignment(0.5, 0.5);
   label13.set_padding(0, 0);
   label14.set_name("label14");
   label14.set_alignment(0.5, 0.5);
   label14.set_padding(0, 0);
   table7.set_name("table7");
   table7.attach(*fxbassamount, 1, 2, 0, 1, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0);
   table7.attach(*fxbassrange, 1, 2, 1, 2, GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0);
   table7.attach(label13, 0, 1, 0, 1, 0, 0, 0, 0);
   table7.attach(label14, 0, 1, 1, 2, 0, 0, 0, 0);
   vbox20.set_name("vbox20");
   vbox20.pack_start(table7);
   hbox11.set_name("hbox11");
   hbox11.pack_start(fxbasson, false, false, 0);
   hbox11.pack_start(vbox20);
   frame14.set_name("frame14");
   frame14.set_label_align(0, 0);
   frame14.set_shadow_type(GTK_SHADOW_ETCHED_IN);
   frame14.add(&hbox11);
   fxsurroundon.set_name("fxSurroundOn");
   fxsurroundon.set_active(true);
   connect_to_method(fxsurroundon.toggled, (CConfigureWindow*)this, &CConfigureWindow::on_fxSurroundOn_toggled);
}

void CConfigureWindow_Glade::init4()
{
   fxsurrounddepth->set_name("fxSurroundDepth");
   fxsurrounddepth->set_digits(1);
   fxsurrounddepth->set_draw_value(false);
   fxsurrounddepth->set_value_pos(GTK_POS_TOP);
//   connect_to_method(fxsurrounddepth->drag_motion, (CConfigureWindow*)this, &CConfigureWindow::on_fxSurroundDepth_drag_motion);
   fxsurrounddelay->set_name("fxSurroundDelay");
   fxsurrounddelay->set_digits(1);
   fxsurrounddelay->set_draw_value(false);
   fxsurrounddelay->set_value_pos(GTK_POS_TOP);
//   connect_to_method(fxsurrounddelay->drag_motion, (CConfigureWindow*)this, &CConfigureWindow::on_fxSurroundDelay_drag_motion);
   label15.set_name("label15");
   label15.set_alignment(0.5, 0.5);
   label15.set_padding(0, 0);
   label16.set_name("label16");
   label16.set_alignment(0.5, 0.5);
   label16.set_padding(0, 0);
   table8.set_name("table8");
   table8.attach(*fxsurrounddepth, 1, 2, 0, 1, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0);
   table8.attach(*fxsurrounddelay, 1, 2, 1, 2, GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0);
   table8.attach(label15, 0, 1, 0, 1, 0, 0, 0, 0);
   table8.attach(label16, 0, 1, 1, 2, 0, 0, 0, 0);
   vbox21.set_name("vbox21");
   vbox21.pack_start(table8);
   hbox12.set_name("hbox12");
   hbox12.pack_start(fxsurroundon, false, false, 0);
   hbox12.pack_start(vbox21);
   frame15.set_name("frame15");
   frame15.set_label_align(0, 0);
   frame15.set_shadow_type(GTK_SHADOW_ETCHED_IN);
   frame15.add(&hbox12);
   fxfadeon.set_name("fxFadeOn");
   fxfadeon.set_active(true);
   connect_to_method(fxfadeon.toggled, (CConfigureWindow*)this, &CConfigureWindow::on_fxFadeOn_toggled);
   label17.set_name("label17");
   label17.set_alignment(0.5, 0.5);
   label17.set_padding(0, 0);
   fxfadetime->set_name("fxFadeTime");
   fxfadetime->set_digits(1);
   fxfadetime->set_draw_value(false);
   fxfadetime->set_value_pos(GTK_POS_TOP);
//   connect_to_method(fxfadetime->button_release_event, (CConfigureWindow*)this, &CConfigureWindow::on_fxFadeTime_drag_motion);
   hbox14.set_name("hbox14");
   hbox14.pack_start(label17, false, false, 0);
   hbox14.pack_start(*fxfadetime);
   vbox22.set_name("vbox22");
   vbox22.pack_start(hbox14);
   hbox13.set_name("hbox13");
   hbox13.pack_start(fxfadeon, false, false, 0);
   hbox13.pack_start(vbox22);
   frame16.set_name("frame16");
   frame16.set_label_align(0, 0);
   frame16.set_shadow_type(GTK_SHADOW_ETCHED_IN);
   frame16.add(&hbox13);
   vbox16.set_name("vbox16");
   vbox16.pack_start(frame12);
   vbox16.pack_start(frame13);
}

void CConfigureWindow_Glade::init5()
{
   vbox16.pack_start(frame14);
   vbox16.pack_start(frame15);
   vbox16.pack_start(frame16);
   label18.set_name("label18");
   label18.set_alignment(0.5, 0.5);
   label18.set_padding(0, 0);
   notebook2.set_name("notebook2");
   notebook2.set_show_tabs(true);
   notebook2.set_tab_border(3);
   notebook2.append_page(vbox12, label10);
   notebook2.append_page(vbox16, label18);
   okbtn.set_usize(56, 24);
   okbtn.set_name("okBtn");
   connect_to_method(okbtn.clicked, (CConfigureWindow*)this, &CConfigureWindow::on_okBtn_clicked);
   applybtn.set_usize(59, 24);
   applybtn.set_name("applyBtn");
   connect_to_method(applybtn.clicked, (CConfigureWindow*)this, &CConfigureWindow::on_applyBtn_clicked);
   cancelbtn.set_usize(59, 24);
   cancelbtn.set_name("cancelBtn");
   connect_to_method(cancelbtn.clicked, (CConfigureWindow*)this, &CConfigureWindow::on_cancelBtn_clicked);
   fixed1.set_name("fixed1");
   fixed1.put(okbtn, 184, 8);
   fixed1.put(applybtn, 256, 8);
   fixed1.put(cancelbtn, 328, 8);
   vbox23.set_name("vbox23");
   vbox23.pack_start(notebook2);
   vbox23.pack_start(fixed1);
   set_name("configure");
   set_title("ModPlug Configuration");
   add(&vbox23);

   bit16->show();
   bit8->show();
   vbox13.show();
   frame9.show();
   stereo->show();
   mono->show();
   vbox14.show();
   frame10.show();
   hbox8.show();
   samp44->show();
   samp22->show();
   samp11->show();
   vbox15.show();
   frame11.show();
   vbox12.show();
   label10.show();
   fxoversamp.show();
   fxnr.show();
   vbox17.show();
   fxvolramp.show();
   fxfastinfo.show();
   vbox18.show();
   hbox9.show();
   frame12.show();
   fxreverbon.show();
   fxreverbdepth->show();
   fxreverbdelay->show();
   label11.show();
   label12.show();
   table6.show();
   vbox19.show();
   hbox10.show();
   frame13.show();
   fxbasson.show();
   fxbassamount->show();
   fxbassrange->show();
   label13.show();
   label14.show();
   table7.show();
   vbox20.show();
   hbox11.show();
   frame14.show();
   fxsurroundon.show();
   fxsurrounddepth->show();
   fxsurrounddelay->show();
   label15.show();
   label16.show();
   table8.show();
   vbox21.show();
   hbox12.show();
   frame15.show();
   fxfadeon.show();
   label17.show();
   fxfadetime->show();
   hbox14.show();
   vbox22.show();
   hbox13.show();
   frame16.show();
   vbox16.show();
   label18.show();
   notebook2.show();
   okbtn.show();
   applybtn.show();
   cancelbtn.show();
   fixed1.show();
   vbox23.show();
   show();
}

CConfigureWindow_Glade::~CConfigureWindow_Glade()
{
   delete bit16;
   delete bit8;
   delete stereo;
   delete mono;
   delete samp44;
   delete samp22;
   delete samp11;
   delete fxreverbdepth;
   delete fxreverbdepth_adj;
   delete fxreverbdelay;
   delete fxreverbdelay_adj;
   delete fxbassamount;
   delete fxbassamount_adj;
   delete fxbassrange;
   delete fxbassrange_adj;
   delete fxsurrounddepth;
   delete fxsurrounddepth_adj;
   delete fxsurrounddelay;
   delete fxsurrounddelay_adj;
   delete fxfadetime;
   delete fxfadetime_adj;
}