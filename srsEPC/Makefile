include ../Environment.mk 

OPFLAG?=-O0

ifeq ($(OPFLAG), -O0)
	SRSRAN_PATH=$(DASH_ROOT)../debug/srsran/
	BOOTH_PATH =$(DASH_ROOT)../debug/boost/
	FFTW_PATH  =$(DASH_ROOT)../debug/fftw/
	SCTP_PATH  =$(DASH_ROOT)../debug/lksctp/
	CONFIG_PATH=$(DASH_ROOT)../debug/libconfig/
	MBED_PATH  =$(DASH_ROOT)../debug/mbedtls/
else
	SRSRAN_PATH=$(DASH_ROOT)../release/srsran/
	BOOST_PATH =$(DASH_ROOT)../release/boost/
	FFTW_PATH  =$(DASH_ROOT)../release/fftw/
	SCTP_PATH  =$(DASH_ROOT)../release/lksctp/
	CONFIG_PATH=$(DASH_ROOT)../release/libconfig/
	MBED_PATH  =$(DASH_ROOT)../release/mbedtls/
endif

SOURCE=srsepc
SUFFIX=.cpp
CCFILES := $(shell find ./ -name '*.cc')
ADDSOURCE=$(CCFILES)
# we need to include jpeg here even though we have the static archive for it because that archive is missing a symbol "qtable"
INCLUDE=-I$(SRSRAN_PATH)include/ -I.
D_LINKS=-lpthread -ldl $(SRSRAN_PATH)lib/libsrsran_rf.so
LIBRARIES=\
$(SRSRAN_PATH)lib/libasn1_utils.a\
$(SRSRAN_PATH)lib/libnas_5g_msg.a\
$(SRSRAN_PATH)lib/libngap_nr_asn1.a\
$(SRSRAN_PATH)lib/librrc_asn1.a\
$(SRSRAN_PATH)lib/librrc_nr_asn1.a\
$(SRSRAN_PATH)lib/libs1ap_asn1.a\
$(SRSRAN_PATH)lib/libsrslog.a\
$(SRSRAN_PATH)lib/libsrsran_asn1.a\
$(SRSRAN_PATH)lib/libsrsran_gtpu.a\
$(SRSRAN_PATH)lib/libsrsran_mac.a\
$(SRSRAN_PATH)lib/libsrsran_pdcp.a\
$(SRSRAN_PATH)lib/libsrsran_phy.a\
$(SRSRAN_PATH)lib/libsrsran_radio.a\
$(SRSRAN_PATH)lib/libsrsran_rlc.a\
$(SRSRAN_PATH)lib/libsrsran_upper.a\
$(SRSRAN_PATH)lib/work/libsrsran_common.a\
$(SRSRAN_PATH)lib/libsupport.a\
$(SRSRAN_PATH)lib/libsystem.a\
$(BOOTH_PATH)lib/libboost_program_options.a\
$(FFTW_PATH)lib/libfftw3f.a\
$(SCTP_PATH)lib/libsctp.a\
$(CONFIG_PATH)lib/libconfig.a\
$(CONFIG_PATH)lib/libconfig++.a\
$(MBED_PATH)lib/libmbedcrypto.a\
$(MBED_PATH)lib/libmbedtls.a\
$(MBED_PATH)lib/libmbedx509.a

RARGS=

POLLY_SHOW=

include ../BuildFlow.mk
