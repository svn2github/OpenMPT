/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

GtkWidget*
create_About ()
{
  GtkWidget *About;
  GtkWidget *vbox1;
  GtkWidget *label1;
  GtkWidget *hseparator1;
  GtkWidget *hbuttonbox1;
  GtkWidget *about_close;

  About = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (About), "About", About);
  gtk_window_set_title (GTK_WINDOW (About), _("About Modplug"));
  gtk_window_set_policy (GTK_WINDOW (About), FALSE, FALSE, FALSE);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox1);
  gtk_object_set_data_full (GTK_OBJECT (About), "vbox1", vbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (About), vbox1);

  label1 = gtk_label_new (_("Modplug Input Plugin for XMMS\nModplug sound engine written by Olivier Lapicque.\nXMMS interface for Modplug by Kenton Varda.\n(c)2000 Olivier Lapicque and Kenton Varda"));
  gtk_widget_ref (label1);
  gtk_object_set_data_full (GTK_OBJECT (About), "label1", label1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (vbox1), label1, FALSE, FALSE, 0);
  gtk_misc_set_padding (GTK_MISC (label1), 6, 6);

  hseparator1 = gtk_hseparator_new ();
  gtk_widget_ref (hseparator1);
  gtk_object_set_data_full (GTK_OBJECT (About), "hseparator1", hseparator1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hseparator1);
  gtk_box_pack_start (GTK_BOX (vbox1), hseparator1, TRUE, TRUE, 0);
  gtk_widget_set_usize (hseparator1, -2, 18);

  hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_widget_ref (hbuttonbox1);
  gtk_object_set_data_full (GTK_OBJECT (About), "hbuttonbox1", hbuttonbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbuttonbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox1, TRUE, TRUE, 0);

  about_close = gtk_button_new_with_label (_("Close"));
  gtk_widget_ref (about_close);
  gtk_object_set_data_full (GTK_OBJECT (About), "about_close", about_close,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (about_close);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), about_close);
  GTK_WIDGET_SET_FLAGS (about_close, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (about_close), "clicked",
                      GTK_SIGNAL_FUNC (on_about_close_clicked),
                      NULL);

  return About;
}

GtkWidget*
create_Config ()
{
  GtkWidget *Config;
  GtkWidget *vbox2;
  GtkWidget *notebook1;
  GtkWidget *vbox3;
  GtkWidget *hbox2;
  GtkWidget *frame1;
  GtkWidget *vbox4;
  GSList *vbox4_group = NULL;
  GtkWidget *bit16;
  GtkWidget *bit8;
  GtkWidget *frame2;
  GtkWidget *vbox5;
  GSList *vbox5_group = NULL;
  GtkWidget *stereo;
  GtkWidget *mono;
  GtkWidget *frame3;
  GtkWidget *vbox6;
  GSList *vbox6_group = NULL;
  GtkWidget *samp44;
  GtkWidget *samp22;
  GtkWidget *samp11;
  GtkWidget *label2;
  GtkWidget *vbox7;
  GtkWidget *frame4;
  GtkWidget *hbox3;
  GtkWidget *vbox8;
  GtkWidget *fxOversamp;
  GtkWidget *fxNR;
  GtkWidget *vbox9;
  GtkWidget *fxVolRamp;
  GtkWidget *fxFastInfo;
  GtkWidget *frame5;
  GtkWidget *hbox4;
  GtkWidget *fxReverb;
  GtkWidget *vbox10;
  GtkWidget *table1;
  GtkWidget *fxReverbDepth;
  GtkWidget *fxReverbDelay;
  GtkWidget *label3;
  GtkWidget *label4;
  GtkWidget *frame6;
  GtkWidget *hbox5;
  GtkWidget *fxBassBoost;
  GtkWidget *vbox11;
  GtkWidget *table2;
  GtkWidget *fxBassAmount;
  GtkWidget *fxBassRange;
  GtkWidget *label5;
  GtkWidget *label6;
  GtkWidget *frame7;
  GtkWidget *hbox6;
  GtkWidget *fxSurround;
  GtkWidget *vbox12;
  GtkWidget *table3;
  GtkWidget *fxSurroundDepth;
  GtkWidget *fxSurroundDelay;
  GtkWidget *label7;
  GtkWidget *label8;
  GtkWidget *frame8;
  GtkWidget *hbox7;
  GtkWidget *fxFadeOut;
  GtkWidget *vbox13;
  GtkWidget *hbox8;
  GtkWidget *label9;
  GtkWidget *fxFadeTime;
  GtkWidget *label10;
  GtkWidget *hbuttonbox2;
  GtkWidget *config_ok;
  GtkWidget *config_apply;
  GtkWidget *config_cancel;

  Config = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (Config), "Config", Config);
  gtk_widget_set_usize (Config, 350, -2);
  gtk_window_set_title (GTK_WINDOW (Config), _("ModPlug Configuration"));
  gtk_window_set_policy (GTK_WINDOW (Config), FALSE, FALSE, FALSE);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox2);
  gtk_object_set_data_full (GTK_OBJECT (Config), "vbox2", vbox2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (Config), vbox2);

  notebook1 = gtk_notebook_new ();
  gtk_widget_ref (notebook1);
  gtk_object_set_data_full (GTK_OBJECT (Config), "notebook1", notebook1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (notebook1);
  gtk_box_pack_start (GTK_BOX (vbox2), notebook1, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (notebook1), 6);

  vbox3 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox3);
  gtk_object_set_data_full (GTK_OBJECT (Config), "vbox3", vbox3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox3);
  gtk_container_add (GTK_CONTAINER (notebook1), vbox3);
  gtk_container_set_border_width (GTK_CONTAINER (vbox3), 6);

  hbox2 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox2);
  gtk_object_set_data_full (GTK_OBJECT (Config), "hbox2", hbox2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox2);
  gtk_box_pack_start (GTK_BOX (vbox3), hbox2, TRUE, TRUE, 0);

  frame1 = gtk_frame_new (_("Resolution"));
  gtk_widget_ref (frame1);
  gtk_object_set_data_full (GTK_OBJECT (Config), "frame1", frame1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame1);
  gtk_box_pack_start (GTK_BOX (hbox2), frame1, TRUE, TRUE, 0);

  vbox4 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox4);
  gtk_object_set_data_full (GTK_OBJECT (Config), "vbox4", vbox4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox4);
  gtk_container_add (GTK_CONTAINER (frame1), vbox4);

  bit16 = gtk_radio_button_new_with_label (vbox4_group, _("16 bit"));
  vbox4_group = gtk_radio_button_group (GTK_RADIO_BUTTON (bit16));
  gtk_widget_ref (bit16);
  gtk_object_set_data_full (GTK_OBJECT (Config), "bit16", bit16,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (bit16);
  gtk_box_pack_start (GTK_BOX (vbox4), bit16, FALSE, FALSE, 0);

  bit8 = gtk_radio_button_new_with_label (vbox4_group, _("8 bit"));
  vbox4_group = gtk_radio_button_group (GTK_RADIO_BUTTON (bit8));
  gtk_widget_ref (bit8);
  gtk_object_set_data_full (GTK_OBJECT (Config), "bit8", bit8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (bit8);
  gtk_box_pack_start (GTK_BOX (vbox4), bit8, FALSE, FALSE, 0);

  frame2 = gtk_frame_new (_("Channels"));
  gtk_widget_ref (frame2);
  gtk_object_set_data_full (GTK_OBJECT (Config), "frame2", frame2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame2);
  gtk_box_pack_start (GTK_BOX (hbox2), frame2, TRUE, TRUE, 0);

  vbox5 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox5);
  gtk_object_set_data_full (GTK_OBJECT (Config), "vbox5", vbox5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox5);
  gtk_container_add (GTK_CONTAINER (frame2), vbox5);

  stereo = gtk_radio_button_new_with_label (vbox5_group, _("Stereo"));
  vbox5_group = gtk_radio_button_group (GTK_RADIO_BUTTON (stereo));
  gtk_widget_ref (stereo);
  gtk_object_set_data_full (GTK_OBJECT (Config), "stereo", stereo,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (stereo);
  gtk_box_pack_start (GTK_BOX (vbox5), stereo, FALSE, FALSE, 0);

  mono = gtk_radio_button_new_with_label (vbox5_group, _("Mono"));
  vbox5_group = gtk_radio_button_group (GTK_RADIO_BUTTON (mono));
  gtk_widget_ref (mono);
  gtk_object_set_data_full (GTK_OBJECT (Config), "mono", mono,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (mono);
  gtk_box_pack_start (GTK_BOX (vbox5), mono, FALSE, FALSE, 0);

  frame3 = gtk_frame_new (_("Sampling Rate"));
  gtk_widget_ref (frame3);
  gtk_object_set_data_full (GTK_OBJECT (Config), "frame3", frame3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame3);
  gtk_box_pack_start (GTK_BOX (vbox3), frame3, TRUE, TRUE, 0);

  vbox6 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox6);
  gtk_object_set_data_full (GTK_OBJECT (Config), "vbox6", vbox6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox6);
  gtk_container_add (GTK_CONTAINER (frame3), vbox6);

  samp44 = gtk_radio_button_new_with_label (vbox6_group, _("44 kHz"));
  vbox6_group = gtk_radio_button_group (GTK_RADIO_BUTTON (samp44));
  gtk_widget_ref (samp44);
  gtk_object_set_data_full (GTK_OBJECT (Config), "samp44", samp44,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (samp44);
  gtk_box_pack_start (GTK_BOX (vbox6), samp44, FALSE, FALSE, 0);

  samp22 = gtk_radio_button_new_with_label (vbox6_group, _("22 kHz"));
  vbox6_group = gtk_radio_button_group (GTK_RADIO_BUTTON (samp22));
  gtk_widget_ref (samp22);
  gtk_object_set_data_full (GTK_OBJECT (Config), "samp22", samp22,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (samp22);
  gtk_box_pack_start (GTK_BOX (vbox6), samp22, FALSE, FALSE, 0);

  samp11 = gtk_radio_button_new_with_label (vbox6_group, _("11 kHz"));
  vbox6_group = gtk_radio_button_group (GTK_RADIO_BUTTON (samp11));
  gtk_widget_ref (samp11);
  gtk_object_set_data_full (GTK_OBJECT (Config), "samp11", samp11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (samp11);
  gtk_box_pack_start (GTK_BOX (vbox6), samp11, FALSE, FALSE, 0);

  label2 = gtk_label_new (_("Quality"));
  gtk_widget_ref (label2);
  gtk_object_set_data_full (GTK_OBJECT (Config), "label2", label2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label2);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), label2);

  vbox7 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox7);
  gtk_object_set_data_full (GTK_OBJECT (Config), "vbox7", vbox7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox7);
  gtk_container_add (GTK_CONTAINER (notebook1), vbox7);
  gtk_container_set_border_width (GTK_CONTAINER (vbox7), 6);

  frame4 = gtk_frame_new (_("General"));
  gtk_widget_ref (frame4);
  gtk_object_set_data_full (GTK_OBJECT (Config), "frame4", frame4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame4);
  gtk_box_pack_start (GTK_BOX (vbox7), frame4, TRUE, TRUE, 0);

  hbox3 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox3);
  gtk_object_set_data_full (GTK_OBJECT (Config), "hbox3", hbox3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox3);
  gtk_container_add (GTK_CONTAINER (frame4), hbox3);

  vbox8 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox8);
  gtk_object_set_data_full (GTK_OBJECT (Config), "vbox8", vbox8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox8);
  gtk_box_pack_start (GTK_BOX (hbox3), vbox8, TRUE, TRUE, 0);

  fxOversamp = gtk_check_button_new_with_label (_("Oversampling"));
  gtk_widget_ref (fxOversamp);
  gtk_object_set_data_full (GTK_OBJECT (Config), "fxOversamp", fxOversamp,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fxOversamp);
  gtk_box_pack_start (GTK_BOX (vbox8), fxOversamp, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fxOversamp), TRUE);

  fxNR = gtk_check_button_new_with_label (_("Noise Reduction"));
  gtk_widget_ref (fxNR);
  gtk_object_set_data_full (GTK_OBJECT (Config), "fxNR", fxNR,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fxNR);
  gtk_box_pack_start (GTK_BOX (vbox8), fxNR, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fxNR), TRUE);

  vbox9 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox9);
  gtk_object_set_data_full (GTK_OBJECT (Config), "vbox9", vbox9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox9);
  gtk_box_pack_start (GTK_BOX (hbox3), vbox9, TRUE, TRUE, 0);

  fxVolRamp = gtk_check_button_new_with_label (_("Volume Ramping"));
  gtk_widget_ref (fxVolRamp);
  gtk_object_set_data_full (GTK_OBJECT (Config), "fxVolRamp", fxVolRamp,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fxVolRamp);
  gtk_box_pack_start (GTK_BOX (vbox9), fxVolRamp, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fxVolRamp), TRUE);

  fxFastInfo = gtk_check_button_new_with_label (_("Fast Playlist Info"));
  gtk_widget_ref (fxFastInfo);
  gtk_object_set_data_full (GTK_OBJECT (Config), "fxFastInfo", fxFastInfo,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fxFastInfo);
  gtk_box_pack_start (GTK_BOX (vbox9), fxFastInfo, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fxFastInfo), TRUE);

  frame5 = gtk_frame_new (_("Reverb"));
  gtk_widget_ref (frame5);
  gtk_object_set_data_full (GTK_OBJECT (Config), "frame5", frame5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame5);
  gtk_box_pack_start (GTK_BOX (vbox7), frame5, TRUE, TRUE, 0);

  hbox4 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox4);
  gtk_object_set_data_full (GTK_OBJECT (Config), "hbox4", hbox4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox4);
  gtk_container_add (GTK_CONTAINER (frame5), hbox4);

  fxReverb = gtk_check_button_new_with_label (_("Enable"));
  gtk_widget_ref (fxReverb);
  gtk_object_set_data_full (GTK_OBJECT (Config), "fxReverb", fxReverb,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fxReverb);
  gtk_box_pack_start (GTK_BOX (hbox4), fxReverb, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fxReverb), TRUE);

  vbox10 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox10);
  gtk_object_set_data_full (GTK_OBJECT (Config), "vbox10", vbox10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox10);
  gtk_box_pack_start (GTK_BOX (hbox4), vbox10, TRUE, TRUE, 0);

  table1 = gtk_table_new (2, 2, FALSE);
  gtk_widget_ref (table1);
  gtk_object_set_data_full (GTK_OBJECT (Config), "table1", table1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table1);
  gtk_box_pack_start (GTK_BOX (vbox10), table1, TRUE, TRUE, 0);

  fxReverbDepth = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 0, 100, 0, 0, 0)));
  gtk_widget_ref (fxReverbDepth);
  gtk_object_set_data_full (GTK_OBJECT (Config), "fxReverbDepth", fxReverbDepth,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fxReverbDepth);
  gtk_table_attach (GTK_TABLE (table1), fxReverbDepth, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  fxReverbDelay = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (100, 40, 200, 0, 0, 0)));
  gtk_widget_ref (fxReverbDelay);
  gtk_object_set_data_full (GTK_OBJECT (Config), "fxReverbDelay", fxReverbDelay,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fxReverbDelay);
  gtk_table_attach (GTK_TABLE (table1), fxReverbDelay, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  label3 = gtk_label_new (_("Depth"));
  gtk_widget_ref (label3);
  gtk_object_set_data_full (GTK_OBJECT (Config), "label3", label3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label3);
  gtk_table_attach (GTK_TABLE (table1), label3, 0, 1, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  label4 = gtk_label_new (_("Delay"));
  gtk_widget_ref (label4);
  gtk_object_set_data_full (GTK_OBJECT (Config), "label4", label4,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label4);
  gtk_table_attach (GTK_TABLE (table1), label4, 0, 1, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  frame6 = gtk_frame_new (_("Bass Boost"));
  gtk_widget_ref (frame6);
  gtk_object_set_data_full (GTK_OBJECT (Config), "frame6", frame6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame6);
  gtk_box_pack_start (GTK_BOX (vbox7), frame6, TRUE, TRUE, 0);

  hbox5 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox5);
  gtk_object_set_data_full (GTK_OBJECT (Config), "hbox5", hbox5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox5);
  gtk_container_add (GTK_CONTAINER (frame6), hbox5);

  fxBassBoost = gtk_check_button_new_with_label (_("Enable"));
  gtk_widget_ref (fxBassBoost);
  gtk_object_set_data_full (GTK_OBJECT (Config), "fxBassBoost", fxBassBoost,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fxBassBoost);
  gtk_box_pack_start (GTK_BOX (hbox5), fxBassBoost, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fxBassBoost), TRUE);

  vbox11 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox11);
  gtk_object_set_data_full (GTK_OBJECT (Config), "vbox11", vbox11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox11);
  gtk_box_pack_start (GTK_BOX (hbox5), vbox11, TRUE, TRUE, 0);

  table2 = gtk_table_new (2, 2, FALSE);
  gtk_widget_ref (table2);
  gtk_object_set_data_full (GTK_OBJECT (Config), "table2", table2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table2);
  gtk_box_pack_start (GTK_BOX (vbox11), table2, TRUE, TRUE, 0);

  fxBassAmount = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (6, 0, 100, 0, 0, 0)));
  gtk_widget_ref (fxBassAmount);
  gtk_object_set_data_full (GTK_OBJECT (Config), "fxBassAmount", fxBassAmount,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fxBassAmount);
  gtk_table_attach (GTK_TABLE (table2), fxBassAmount, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  fxBassRange = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (14, 10, 100, 0, 0, 0)));
  gtk_widget_ref (fxBassRange);
  gtk_object_set_data_full (GTK_OBJECT (Config), "fxBassRange", fxBassRange,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fxBassRange);
  gtk_table_attach (GTK_TABLE (table2), fxBassRange, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  label5 = gtk_label_new (_("Amount"));
  gtk_widget_ref (label5);
  gtk_object_set_data_full (GTK_OBJECT (Config), "label5", label5,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label5);
  gtk_table_attach (GTK_TABLE (table2), label5, 0, 1, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  label6 = gtk_label_new (_("Range"));
  gtk_widget_ref (label6);
  gtk_object_set_data_full (GTK_OBJECT (Config), "label6", label6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label6);
  gtk_table_attach (GTK_TABLE (table2), label6, 0, 1, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  frame7 = gtk_frame_new (_("Surround"));
  gtk_widget_ref (frame7);
  gtk_object_set_data_full (GTK_OBJECT (Config), "frame7", frame7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame7);
  gtk_box_pack_start (GTK_BOX (vbox7), frame7, TRUE, TRUE, 0);

  hbox6 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox6);
  gtk_object_set_data_full (GTK_OBJECT (Config), "hbox6", hbox6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox6);
  gtk_container_add (GTK_CONTAINER (frame7), hbox6);

  fxSurround = gtk_check_button_new_with_label (_("Enable"));
  gtk_widget_ref (fxSurround);
  gtk_object_set_data_full (GTK_OBJECT (Config), "fxSurround", fxSurround,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fxSurround);
  gtk_box_pack_start (GTK_BOX (hbox6), fxSurround, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fxSurround), TRUE);

  vbox12 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox12);
  gtk_object_set_data_full (GTK_OBJECT (Config), "vbox12", vbox12,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox12);
  gtk_box_pack_start (GTK_BOX (hbox6), vbox12, TRUE, TRUE, 0);

  table3 = gtk_table_new (2, 2, FALSE);
  gtk_widget_ref (table3);
  gtk_object_set_data_full (GTK_OBJECT (Config), "table3", table3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table3);
  gtk_box_pack_start (GTK_BOX (vbox12), table3, TRUE, TRUE, 0);

  fxSurroundDepth = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (12, 0, 100, 0, 0, 0)));
  gtk_widget_ref (fxSurroundDepth);
  gtk_object_set_data_full (GTK_OBJECT (Config), "fxSurroundDepth", fxSurroundDepth,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fxSurroundDepth);
  gtk_table_attach (GTK_TABLE (table3), fxSurroundDepth, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  fxSurroundDelay = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (20, 5, 40, 0, 0, 0)));
  gtk_widget_ref (fxSurroundDelay);
  gtk_object_set_data_full (GTK_OBJECT (Config), "fxSurroundDelay", fxSurroundDelay,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fxSurroundDelay);
  gtk_table_attach (GTK_TABLE (table3), fxSurroundDelay, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  label7 = gtk_label_new (_("Depth"));
  gtk_widget_ref (label7);
  gtk_object_set_data_full (GTK_OBJECT (Config), "label7", label7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label7);
  gtk_table_attach (GTK_TABLE (table3), label7, 0, 1, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  label8 = gtk_label_new (_("Delay"));
  gtk_widget_ref (label8);
  gtk_object_set_data_full (GTK_OBJECT (Config), "label8", label8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label8);
  gtk_table_attach (GTK_TABLE (table3), label8, 0, 1, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  frame8 = gtk_frame_new (_("Fade on Stop (WARINING: SEE README)"));
  gtk_widget_ref (frame8);
  gtk_object_set_data_full (GTK_OBJECT (Config), "frame8", frame8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (frame8);
  gtk_box_pack_start (GTK_BOX (vbox7), frame8, TRUE, TRUE, 0);

  hbox7 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox7);
  gtk_object_set_data_full (GTK_OBJECT (Config), "hbox7", hbox7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox7);
  gtk_container_add (GTK_CONTAINER (frame8), hbox7);

  fxFadeOut = gtk_check_button_new_with_label (_("Enable"));
  gtk_widget_ref (fxFadeOut);
  gtk_object_set_data_full (GTK_OBJECT (Config), "fxFadeOut", fxFadeOut,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fxFadeOut);
  gtk_box_pack_start (GTK_BOX (hbox7), fxFadeOut, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fxFadeOut), TRUE);

  vbox13 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox13);
  gtk_object_set_data_full (GTK_OBJECT (Config), "vbox13", vbox13,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox13);
  gtk_box_pack_start (GTK_BOX (hbox7), vbox13, TRUE, TRUE, 0);

  hbox8 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox8);
  gtk_object_set_data_full (GTK_OBJECT (Config), "hbox8", hbox8,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox8);
  gtk_box_pack_start (GTK_BOX (vbox13), hbox8, TRUE, TRUE, 0);

  label9 = gtk_label_new (_("Length\n(ms)"));
  gtk_widget_ref (label9);
  gtk_object_set_data_full (GTK_OBJECT (Config), "label9", label9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label9);
  gtk_box_pack_start (GTK_BOX (hbox8), label9, FALSE, FALSE, 0);

  fxFadeTime = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (500, 0, 2000, 0, 0, 0)));
  gtk_widget_ref (fxFadeTime);
  gtk_object_set_data_full (GTK_OBJECT (Config), "fxFadeTime", fxFadeTime,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (fxFadeTime);
  gtk_box_pack_start (GTK_BOX (hbox8), fxFadeTime, TRUE, TRUE, 0);

  label10 = gtk_label_new (_("Effects"));
  gtk_widget_ref (label10);
  gtk_object_set_data_full (GTK_OBJECT (Config), "label10", label10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label10);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), label10);

  hbuttonbox2 = gtk_hbutton_box_new ();
  gtk_widget_ref (hbuttonbox2);
  gtk_object_set_data_full (GTK_OBJECT (Config), "hbuttonbox2", hbuttonbox2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbuttonbox2);
  gtk_box_pack_start (GTK_BOX (vbox2), hbuttonbox2, TRUE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox2), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonbox2), 0);

  config_ok = gtk_button_new_with_label (_("OK"));
  gtk_widget_ref (config_ok);
  gtk_object_set_data_full (GTK_OBJECT (Config), "config_ok", config_ok,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (config_ok);
  gtk_container_add (GTK_CONTAINER (hbuttonbox2), config_ok);
  GTK_WIDGET_SET_FLAGS (config_ok, GTK_CAN_DEFAULT);

  config_apply = gtk_button_new_with_label (_("Apply"));
  gtk_widget_ref (config_apply);
  gtk_object_set_data_full (GTK_OBJECT (Config), "config_apply", config_apply,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (config_apply);
  gtk_container_add (GTK_CONTAINER (hbuttonbox2), config_apply);
  GTK_WIDGET_SET_FLAGS (config_apply, GTK_CAN_DEFAULT);

  config_cancel = gtk_button_new_with_label (_("Cancel"));
  gtk_widget_ref (config_cancel);
  gtk_object_set_data_full (GTK_OBJECT (Config), "config_cancel", config_cancel,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (config_cancel);
  gtk_container_add (GTK_CONTAINER (hbuttonbox2), config_cancel);
  GTK_WIDGET_SET_FLAGS (config_cancel, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (config_ok), "clicked",
                      GTK_SIGNAL_FUNC (on_config_ok_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (config_apply), "clicked",
                      GTK_SIGNAL_FUNC (on_config_apply_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (config_cancel), "clicked",
                      GTK_SIGNAL_FUNC (on_config_cancel_clicked),
                      NULL);

  return Config;
}

GtkWidget*
create_Info ()
{
  GtkWidget *Info;
  GtkWidget *vbox14;
  GtkWidget *notebook2;
  GtkWidget *hbox9;
  GtkWidget *label11;
  GtkWidget *info_general;
  GtkWidget *label13;
  GtkWidget *scrolledwindow1;
  GtkWidget *viewport1;
  GtkWidget *info_samples;
  GtkWidget *label15;
  GtkWidget *scrolledwindow2;
  GtkWidget *viewport2;
  GtkWidget *info_instruments;
  GtkWidget *label17;
  GtkWidget *hbuttonbox3;
  GtkWidget *info_close;

  Info = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (Info), "Info", Info);
  gtk_widget_set_usize (Info, 290, 240);
  gtk_window_set_title (GTK_WINDOW (Info), _("MOD Info"));
  gtk_window_set_policy (GTK_WINDOW (Info), FALSE, FALSE, FALSE);

  vbox14 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox14);
  gtk_object_set_data_full (GTK_OBJECT (Info), "vbox14", vbox14,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox14);
  gtk_container_add (GTK_CONTAINER (Info), vbox14);

  notebook2 = gtk_notebook_new ();
  gtk_widget_ref (notebook2);
  gtk_object_set_data_full (GTK_OBJECT (Info), "notebook2", notebook2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (notebook2);
  gtk_box_pack_start (GTK_BOX (vbox14), notebook2, TRUE, TRUE, 8);
  gtk_container_set_border_width (GTK_CONTAINER (notebook2), 6);

  hbox9 = gtk_hbox_new (FALSE, 0);
  gtk_widget_ref (hbox9);
  gtk_object_set_data_full (GTK_OBJECT (Info), "hbox9", hbox9,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbox9);
  gtk_container_add (GTK_CONTAINER (notebook2), hbox9);

  label11 = gtk_label_new (_("Filename:\nTitle:\nType:\nLength:\nSpeed:\nTempo:\nSamples:\nInstruments:\nPatterns:\nChannels:"));
  gtk_widget_ref (label11);
  gtk_object_set_data_full (GTK_OBJECT (Info), "label11", label11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label11);
  gtk_box_pack_start (GTK_BOX (hbox9), label11, FALSE, FALSE, 4);
  gtk_label_set_justify (GTK_LABEL (label11), GTK_JUSTIFY_LEFT);

  info_general = gtk_label_new (_("---\n---\n---\n---\n---\n---\n---\n---\n---\n---"));
  gtk_widget_ref (info_general);
  gtk_object_set_data_full (GTK_OBJECT (Info), "info_general", info_general,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (info_general);
  gtk_box_pack_start (GTK_BOX (hbox9), info_general, FALSE, FALSE, 4);
  gtk_label_set_justify (GTK_LABEL (info_general), GTK_JUSTIFY_LEFT);

  label13 = gtk_label_new (_("General"));
  gtk_widget_ref (label13);
  gtk_object_set_data_full (GTK_OBJECT (Info), "label13", label13,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label13);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook2), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook2), 0), label13);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_ref (scrolledwindow1);
  gtk_object_set_data_full (GTK_OBJECT (Info), "scrolledwindow1", scrolledwindow1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow1);
  gtk_container_add (GTK_CONTAINER (notebook2), scrolledwindow1);
  gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow1), 6);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  viewport1 = gtk_viewport_new (NULL, NULL);
  gtk_widget_ref (viewport1);
  gtk_object_set_data_full (GTK_OBJECT (Info), "viewport1", viewport1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (viewport1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), viewport1);

  info_samples = gtk_label_new ("");
  gtk_widget_ref (info_samples);
  gtk_object_set_data_full (GTK_OBJECT (Info), "info_samples", info_samples,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (info_samples);
  gtk_container_add (GTK_CONTAINER (viewport1), info_samples);
  gtk_label_set_justify (GTK_LABEL (info_samples), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (info_samples), 7.45058e-09, 7.45058e-09);

  label15 = gtk_label_new (_("Samples"));
  gtk_widget_ref (label15);
  gtk_object_set_data_full (GTK_OBJECT (Info), "label15", label15,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label15);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook2), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook2), 1), label15);

  scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_ref (scrolledwindow2);
  gtk_object_set_data_full (GTK_OBJECT (Info), "scrolledwindow2", scrolledwindow2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow2);
  gtk_container_add (GTK_CONTAINER (notebook2), scrolledwindow2);
  gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow2), 6);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  viewport2 = gtk_viewport_new (NULL, NULL);
  gtk_widget_ref (viewport2);
  gtk_object_set_data_full (GTK_OBJECT (Info), "viewport2", viewport2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (viewport2);
  gtk_container_add (GTK_CONTAINER (scrolledwindow2), viewport2);

  info_instruments = gtk_label_new ("");
  gtk_widget_ref (info_instruments);
  gtk_object_set_data_full (GTK_OBJECT (Info), "info_instruments", info_instruments,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (info_instruments);
  gtk_container_add (GTK_CONTAINER (viewport2), info_instruments);
  gtk_label_set_justify (GTK_LABEL (info_instruments), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (info_instruments), 1.49012e-08, 7.45058e-09);

  label17 = gtk_label_new (_("Instruments"));
  gtk_widget_ref (label17);
  gtk_object_set_data_full (GTK_OBJECT (Info), "label17", label17,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label17);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook2), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook2), 2), label17);

  hbuttonbox3 = gtk_hbutton_box_new ();
  gtk_widget_ref (hbuttonbox3);
  gtk_object_set_data_full (GTK_OBJECT (Info), "hbuttonbox3", hbuttonbox3,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (hbuttonbox3);
  gtk_box_pack_start (GTK_BOX (vbox14), hbuttonbox3, FALSE, FALSE, 0);

  info_close = gtk_button_new_with_label (_("Close"));
  gtk_widget_ref (info_close);
  gtk_object_set_data_full (GTK_OBJECT (Info), "info_close", info_close,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (info_close);
  gtk_container_add (GTK_CONTAINER (hbuttonbox3), info_close);
  GTK_WIDGET_SET_FLAGS (info_close, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (info_close), "clicked",
                      GTK_SIGNAL_FUNC (on_info_close_clicked),
                      NULL);

  return Info;
}
