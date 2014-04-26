# name of your application
export PROJECT = helloHA

# If no BOARD is found in the environment, use this default:
export BOARD ?= iot-lab_M3

# This has to be the absolute path to the RIOT base directory:
export RIOTBASE ?= $(CURDIR)/../../RIOT

# Uncomment these lines if you want to use platform support from external
# repositories:
export RIOTCPU ?= $(CURDIR)/../../thirdparty_cpu
export RIOTBOARD ?= $(CURDIR)/../../thirdparty_boards

# Uncomment this to enable scheduler statistics for ps:
#CFLAGS += -DSCHEDSTATISTICS

# If you want to use native with valgrind, you should recompile native
# with the target all-valgrind instead of all:
# make -B clean all-valgrind

# Uncomment this to enable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
export QUIET ?= 0

#VPATH and SRCLOC
SRCLOC = MBoard1-libs/
INCLOC = MBoard1-libs/
VPATH = $(SRCLOC)

# Modules to include:

USEMODULE += posix
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += vtimer
USEMODULE += auto_init
ifneq (,$(filter msb-430,$(BOARD)))
	USEMODULE += sht11
endif
ifneq (,$(filter msb-430h,$(BOARD)))
	USEMODULE += sht11
	USEMODULE += cc110x_ng
endif
ifneq (,$(filter chronos,$(BOARD)))
	USEMODULE += cc110x_ng
endif
ifneq (,$(filter wsn430-v1_3b,$(BOARD)))
	USEMODULE += cc110x_ng
endif
ifneq (,$(filter msba2,$(BOARD)))
	USEMODULE += sht11
	USEMODULE += ltc4150
	USEMODULE += rtc
	USEMODULE += mci
	USEMODULE += cc110x_ng
	USEMODULE += config
	USEMODULE += random
endif
ifneq (,$(filter native,$(BOARD)))
	USEMODULE += ltc4150
	USEMODULE += rtc
	USEMODULE += nativenet
	USEMODULE += transceiver
	USEMODULE += config
	USEMODULE += random
endif

export INCLUDES += -I${RIOTBASE}/core/include/ -I${RIOTBASE}/sys/include/ -I${RIOTBASE}/drivers/include/ $(addprefix -I${CURDIR}/, $(INCLOC))

include $(RIOTBASE)/Makefile.include
