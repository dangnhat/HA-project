# name of your application
export APPLICATION ?= hello_HA

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
# CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
export QUIET ?= 0

#VPATH and SRCLOC
SRCLOC = MBoard1-libs/ TuLib/Src/
INCLOC = MBoard1-libs/ TuLib/Inc/
VPATH = $(SRCLOC)

#HA specific flags
CXXEXFLAGS += -fno-exceptions -fno-rtti -std=gnu++11
CFLAGS += -ffunction-sections -fdata-sections
LINKFLAGS += -Wl,--gc-sections

# Modules to include:
USEMODULE += posix
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += vtimer
USEMODULE += defaulttransceiver
USEMODULE += auto_init

export INCLUDES += $(addprefix -I${CURDIR}/, $(INCLOC))

include $(RIOTBASE)/Makefile.include
