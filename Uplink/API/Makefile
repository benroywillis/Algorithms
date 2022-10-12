include ../../Environment.mk


SOURCE=metrics_test
SUFFIX=.cpp

INCLUDE=-I$(SRSRAN_ROOT)/include/ \
-I$(BWILLI_ROOT)/../Builds/srsRAN-release_22_04_1/srsue/hdr/

LIBRARIES=\
$(SRSRAN_ROOT)lib/libasn1_utils.a\
$(SRSRAN_ROOT)lib/libnas_5g_msg.a\
$(SRSRAN_ROOT)lib/librrc_asn1.a\
$(SRSRAN_ROOT)lib/libsrslog.a\
$(SRSRAN_ROOT)lib/libsrsran_mac.a\
$(SRSRAN_ROOT)lib/libsrsran_radio.a\
$(SRSRAN_ROOT)lib/libsrsran_rlc.a\
$(SRSRAN_ROOT)lib/librrc_nr_asn1.a\
$(SRSRAN_ROOT)lib/libsrsran_pdcp.a\
$(SRSRAN_ROOT)lib/libngap_nr_asn1.a\
$(SRSRAN_ROOT)lib/libs1ap_asn1.a\
$(SRSRAN_ROOT)lib/libsrsran_gtpu.a\
$(SRSRAN_ROOT)lib/libsrsran_phy.a
#$(SRSRAN_ROOT)lib/libsrsran_common.a\

RARGS=-o output.csv

include ../../BuildFlow.mk
