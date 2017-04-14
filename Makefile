RM     := /bin/rm -rf

buffer:
	g++ -std=c++11 -g -pthread mmp_init.cc mmp_thread.cc mmp_user.cc vecs.cc -o xvecs_buffer

base:
	g++ -std=c++11 -g base_vecs.cc -o xvecs_base

#thread_buffer:
#	gcc -O0 -g thread_buffer.c mmp_user.c mmp_init.c mmp_thread.c -o xthread_buff -lpthread

#thread_base:
#	gcc -O0 -g thread_base.c -o xthread_base -lpthread

clean:
	$(RM) xvecs_buffer xvecs_base

