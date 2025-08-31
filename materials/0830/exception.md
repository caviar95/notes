#0 in pthread_join() from libpthread.so
#1 in service::AServiceStub::~AServiceStub
#2 in std::default_delete<service::AServiceStub>::operator()
#3 in std::unique_ptr<service::AServiceStub, std::default_delete<service::AServiceStub>::~unique_ptr
#4 in Buisiness::ClientAService::Instance()::ins+168>
#5 in __run_exit_handlers() from libc.so.6
#6 in exit()
#7 in __libc_start_main()
#8 in _start()
