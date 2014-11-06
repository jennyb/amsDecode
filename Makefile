
TARGETS=amsDecode licenseCheck processOpsRoom

default: all

all: $(TARGETS)

clean: 
	rm $(TARGETS) *.o

licenseCheck: licenseCheck.o ngr2ll.o licenseProcessing.o 
	$(CC) -g  -o licenseCheck licenseCheck.o ngr2ll.o licenseProcessing.o -lm

amsDecode: amsDecode.o ngr2ll.o licenseProcessing.o 
	$(CC) -o amsDecode amsDecode.o ngr2ll.o licenseProcessing.o -lm

processOpsRoom: processOpsRoom.o  
	$(CC) -o processOpsRoom processOpsRoom.o 

.c.o:
	$(CC) -c $<


