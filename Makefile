RM     := /bin/rm -rf
BOOST  :=-lboost_program_options -lboost_random
DFLAG	 :=OR
FP		 :=-fpermissive
JUDY	 :=-lJudy

buffer:
	g++ -std=c++14 -g -pthread -fpermissive mmp_init.cc mmp_thread.cc mmp_user.cc vecs.cc -o xvecs_buffer

lat_buffer:
	g++ -std=c++14 -g -D$(DFLAG) $(FP) -pthread mmp_init.cc mmp_thread.cc mmp_user.cc vecs.cc -o lat_bufferc $(JUDY)

base:
	g++ -std=c++11 -g base_vecs.cc -o xvecs_base

base_log:
	g++ -std=c++11 -g p_vecs.cc -o log_base

sps:
	g++ -std=c++11 -g -D$(DFLAG) -pthread mmp_init.cc mmp_thread.cc mmp_user.cc sps.cc -o sps

hashtable:
	g++ -std=c++11 -g -D$(DFLAG) -pthread mmp_init.cc mmp_thread.cc mmp_user.cc hash.cc -o hashtable

#thread_buffer:
#	gcc -O0 -g thread_buffer.c mmp_user.c mmp_init.c mmp_thread.c -o xthread_buff -lpthread

#thread_base:
#	gcc -O0 -g thread_base.c -o xthread_base -lpthread

clean:
	$(RM) xvecs_buffer xvecs_base sps hashtable lat_buffer log_base
