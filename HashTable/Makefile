RM     := /bin/rm -rf
BOOST  :=-lboost_program_options -lboost_random
DFLAG	 :=OR

hashtable:
	g++ -std=c++11 -g -D$(DFLAG) -pthread ../mmp_init.cc ../mmp_thread.cc ../mmp_user.cc hash.cc -o hashtable
buffer:
	g++ -std=c++11 -g -D$(DFLAG) -pthread ../mmp_init.cc ../mmp_thread.cc ../mmp_user.cc hash_buffer.cc -o buffer
#thread_buffer:
#	gcc -O0 -g thread_buffer.c mmp_user.c mmp_init.c mmp_thread.c -o xthread_buff -lpthread

#thread_base:
#	gcc -O0 -g thread_base.c -o xthread_base -lpthread

clean:
	$(RM) hashtable buffer
