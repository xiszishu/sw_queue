RM     := /bin/rm -rf
BOOST  :=-lboost_program_options -lboost_random
DFLAG	 := OR
F			 :=-fpermissive
JUDY	 :=-lJudy

btree:
	g++ -std=c++11 -g -D$(DFLAG) -pthread $(F) btree.cc -o btree $(JUDY)
buffer:
	g++ -std=c++11 -g -D$(DFLAG) -pthread $(F) ../mmp_init.cc ../mmp_thread.cc ../mmp_user.cc buffer_btree.cc -o buffer $(JUDY)

#thread_buffer:
#	gcc -O0 -g thread_buffer.c mmp_user.c mmp_init.c mmp_thread.c -o xthread_buff -lpthread

#thread_base:
#	gcc -O0 -g thread_base.c -o xthread_base -lpthread

clean:
	$(RM) btree buffer
