
TARGET = ldo_index memory speed
all: check $(TARGET)

NIMBUS_ROOT = ../..
include ${NIMBUS_ROOT}/Makeinclude

IFLAGS += -I$(LDIR)
H_FILES = $(wildcard *.h)
CPP_FILES = $(wildcard *.cc)
OBJECT_FILES = $(subst .cc,.o,$(CPP_FILES))
 
LFLAGS += -lnimbus -lboost_thread-mt -lboost_system-mt -lpthread -lprotobuf
LDFLAGS += -L.

# Add the dependencies here
%.o: %.cc
	$(CPP) $(CFLAGS) $(IFLAGS) -c $< -o $@ 

ldo_index: $(CPP_FILES) $(H_FILES) $(OBJECT_FILES)
	$(CPP) $(CFLAGS) $(IFLAGS) $(LDFLAGS) $(LFLAGS) main.o -o $@ $(LFLAGS)

memory: $(CPP_FILES) $(H_FILES) $(OBJECT_FILES)
	$(CPP) $(CFLAGS) $(IFLAGS) $(LDFLAGS) $(LFLAGS) memory.o -o $@ $(LFLAGS)

clean: clean-files
	\rm -f $(TARGET)

speed:  $(CPP_FILES) $(H_FILES) $(OBJECT_FILES)
	$(CPP) $(CFLAGS) $(IFLAGS) $(LDFLAGS) $(LFLAGS) speed.o -o $@ $(LFLAGS)
