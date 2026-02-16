#############################################################################
#
# Definitions
#
#############################################################################

CFLAGS = -D_GNU_SOURCE -std=c11 -g -Wall -Werror -O3
LIBS = -lpthread -lm
COMMON = util.o random.o test_list.o main.c 
COMMONLIST = list.h locks/picklock.h list_lock.c

#############################################################################
#
# Main targets
#
#############################################################################



all: test_list_spinlock test_list_mutex test_list_ticketlock test_list_mcslock \
	test_list_mcslock_custom

clean:
	touch nofail.o
	touch nofail.exe
	rm -f *.o *.exe *~ locks/*~

#############################################################################
#
# Utility stuff
#
#############################################################################

random.o: random.h random.c test.h
	gcc $(CFLAGS) -c -o random.o random.c

util.o: util.h util.c
	gcc $(CFLAGS) -c -o util.o util.c

#############################################################################
#
# Test "driver" code
#
#############################################################################

test_list.o: test.h list.h test_list.c
	gcc $(CFLAGS) -c -o test_list.o test_list.c

#############################################################################
#
# Implementations of specific list/lock scheme pairings.
#
#############################################################################

list_mutex.o: $(COMMONLIST)
	gcc $(CFLAGS) -DPTHREADMUTEX -c -o list_mutex.o list_lock.c

list_spinlock.o: locks/spinlock.h $(COMMONLIST)
	gcc $(CFLAGS) -DSPINLOCK -c -o list_spinlock.o list_lock.c

list_ticketlock.o: locks/ticketlock.h $(COMMONLIST)
	gcc $(CFLAGS) -DTICKETLOCK -c -o list_ticketlock.o list_lock.c

list_mcslock.o: locks/mcslock.h $(COMMONLIST)
	gcc $(CFLAGS) -DMCSLOCK -c -o list_mcslock.o list_lock.c

list_mcslock_custom.o: list.h locks/mcslock.h list_mcslock.c
	gcc $(CFLAGS) -DMCSLOCK -c -o list_mcslock_custom.o list_mcslock.c

#############################################################################
#
# Linking of executables.
#
#############################################################################

# Lists

test_list_mutex: list_mutex.o $(COMMON)
	gcc $(CFLAGS) -o test_list_mutex.exe list_mutex.o $(COMMON) $(LIBS)

test_list_spinlock: list_spinlock.o $(COMMON)
	gcc $(CFLAGS) -o test_list_spinlock.exe list_spinlock.o $(COMMON) $(LIBS)

test_list_ticketlock: list_ticketlock.o $(COMMON)
	gcc $(CFLAGS) -o test_list_ticketlock.exe list_ticketlock.o $(COMMON) $(LIBS)

test_list_mcslock: list_mcslock.o $(COMMON)
	gcc $(CFLAGS) -o test_list_mcslock.exe list_mcslock.o $(COMMON) $(LIBS)

test_list_mcslock_custom: list_mcslock_custom.o $(COMMON)
	gcc $(CFLAGS) -o test_list_mcslock_custom.exe list_mcslock_custom.o $(COMMON) $(LIBS)

