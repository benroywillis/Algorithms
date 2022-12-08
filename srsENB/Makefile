include ../Environment.mk 

OPFLAG?=-O0

ifeq ($(OPFLAG), -O0)
	SRSRAN_PATH=$(DASH_ROOT)../debug/srsran/
	BOOTH_PATH =$(DASH_ROOT)../debug/boost/
	FFTW_PATH  =$(DASH_ROOT)../debug/fftw/
else
	SRSRAN_PATH=$(DASH_ROOT)../release/srsran/
	BOOST_PATH =$(DASH_ROOT)../release/boost/
	FFTW_PATH  =$(DASH_ROOT)../release/fftw/
endif

SOURCE=srsenb
SUFFIX=.cpp
CCFILES := $(shell find ./ -name '*.cc')
ADDSOURCE=$(CCFILES)
# we need to include jpeg here even though we have the static archive for it because that archive is missing a symbol "qtable"
INCLUDE=-I$(SRSRAN_PATH)include/ -I.
D_LINKS=-lpthread
LIBRARIES=\
$(SRSRAN_PATH)lib/libasn1_utils.a\
$(SRSRAN_PATH)lib/libnas_5g_msg.a\
$(SRSRAN_PATH)lib/libngap_nr_asn1.a\
$(SRSRAN_PATH)lib/librrc_asn1.a\
$(SRSRAN_PATH)lib/librrc_nr_asn1.a\
$(SRSRAN_PATH)lib/libs1ap_asn1.a\
$(SRSRAN_PATH)lib/libsrslog.a\
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
#$(SRSRAN_PATH)lib/libsrsran_common.a\

RARGS=

POLLY_SHOW=

include ../BuildFlow.mk
