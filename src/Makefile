LDLIBS=-levdev -lgpiod
EXEC=ptc_qt1_self_demo ptc_qt1_mutual_demo ptc_qt2_mutual_demo ptc_qt6_mutual_demo

all: $(EXEC)

ptc_qt1_mutual_demo: ptc_qt1_mutual.o ptc_qt.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

ptc_qt1_self_demo: ptc_qt1_self.o ptc_qt.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

ptc_qt1_self.o: ptc_qt1.c ptc_qt.h
	$(CC) $(CFLAGS) -o $@ -c $< -D SELFCAP

ptc_qt1_mutual.o: ptc_qt1.c ptc_qt.h
	$(CC) $(CFLAGS) -o $@ -c $< -D MUTCAP

ptc_qt2_mutual_demo: ptc_qt2.o ptc_qt.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

ptc_qt2.o: ptc_qt.h

ptc_qt6_mutual_demo: ptc_qt6.o ptc_qt.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

ptc_qt6.o: ptc_qt.h

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean mrpoper
clean:
	rm -f *.o

mrproper: clean
	rm -rf $(EXEC)
