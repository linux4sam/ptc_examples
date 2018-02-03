LDLIBS=-levdev -lgpiod
EXEC=atqt1_sc_demo atqt1_mc_demo atqt2_demo atqt6_demo

all: $(EXEC)

atqt1_mc_demo: atqt1_mc.o atqt.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

atqt1_sc_demo: atqt1_sc.o atqt.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

atqt1_sc.o: atqt1.c atqt.h
	$(CC) $(CFLAGS) -o $@ -c $< -D SELFCAP

atqt1_mc.o: atqt1.c atqt.h
	$(CC) $(CFLAGS) -o $@ -c $< -D MUTCAP

atqt2_demo: atqt2.o atqt.h atqt.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

atqt2.o: atqt.h

atqt6_demo: atqt6.o atqt.h atqt.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

atqt6.o: atqt.h

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean mrpoper
clean:
	rm -f *.o

mrproper: clean
	rm -rf $(EXEC)
