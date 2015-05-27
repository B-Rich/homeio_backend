HomeIO::HomeIO() {
  measTypeArray = new MeasTypeArray;
  measFetcher = new MeasFetcher;
  ioProxy = new IoProxy;
  tcpServer = new TcpServer;
  tcpCommand = new TcpCommand;
  ioServer = new IoServer;
  actionTypeArray = new ActionTypeArray;
  overseerArray = new OverseerArray;
  fileStorage = new FileStorage;
  measBufferBackupStorage = new MeasBufferBackupStorage;
  frontendSettings = new FrontendSettings;
  spy = new Spy;
  
  ioServerReady = false;
  
  // setup some variables
  measFetcher->measTypeArray = measTypeArray;
  measFetcher->ioProxy = ioProxy;
  
  tcpCommand->measTypeArray = measTypeArray;
  tcpCommand->measFetcher = measFetcher;
  tcpCommand->actionTypeArray = actionTypeArray;
  tcpCommand->overseerArray = overseerArray;
  tcpCommand->frontendSettings = frontendSettings;
  tcpServer->tcpCommand = tcpCommand;
  
  fileStorage->measTypeArray = measTypeArray;
  measBufferBackupStorage->measTypeArray = measTypeArray;
  spy->measTypeArray = measTypeArray;
}

unsigned char HomeIO::startFetch() {
  // set IoProxy to measurements
  for(std::vector<MeasType>::iterator m = measTypeArray->measTypes.begin(); m != measTypeArray->measTypes.end(); ++m) {
    m->ioProxy = ioProxy;
  }
  
  measFetcher->start();
  
  return 0;
}

unsigned char HomeIO::startServer() {
  tcpServer->start();
  
  return 0;
}

unsigned char HomeIO::startIoServer() {
  ioServer->start();
  
  return 0;
}

unsigned char HomeIO::startOverseer() {
  // set IoProxy to actions
  for(std::vector<ActionType>::iterator a = actionTypeArray->actionTypes.begin(); a != actionTypeArray->actionTypes.end(); ++a) {
    a->ioProxy = ioProxy;
  }

  // access is needed to search for proper objects
  overseerArray->measTypeArray = measTypeArray;
  overseerArray->actionTypeArray = actionTypeArray;
  overseerArray->start();
  
  return 0;
}

unsigned char HomeIO::startFileStorage() {
  fileStorage->start();
  
  return 0;
}

unsigned char HomeIO::startBufferBackupStorage() {
  measBufferBackupStorage->start();
  
  return 0;
}

unsigned char HomeIO::startSpy() {
  spy->start();
  
  return 0;
}

void *measStartThread(void *argument)
{
  logInfo("Thread: startFetch() - meas fetching");

  HomeIO *h = (HomeIO *) argument;
  h->startFetch();
  
  return NULL;
}

void *tcpServerThread(void *argument)
{
  logInfo("Thread: startServer() - TCP commands");

  HomeIO *h = (HomeIO *) argument;
  h->startServer();
}

void *ioServerThread(void *argument)
{
  logInfo("Thread: startIoServer() - IoServer - hardware-TCP bridge");

  HomeIO *h = (HomeIO *) argument;
  h->startIoServer();
}

void *ioOverseerThread(void *argument)
{
  logInfo("Thread: ioOverseerThread() - low level overseeers");

  HomeIO *h = (HomeIO *) argument;
  h->startOverseer();
}

void *fileStorageThread(void *argument)
{
  logInfo("Thread: fileStorageThread() - store measurement in files");

  HomeIO *h = (HomeIO *) argument;
  h->startFileStorage();
}

void *fileBufferBackupThread(void *argument)
{
  logInfo("Thread: fileBufferBackupThread() - store measurement buffer backup");

  HomeIO *h = (HomeIO *) argument;
  h->startBufferBackupStorage();
}

void *spyThread(void *argument)
{
  logInfo("Thread: spyThread() - announce measurements to central server");

  HomeIO *h = (HomeIO *) argument;
  h->startSpy();
}

void HomeIO::copyInternalDelays() {
  fileStorage->usDelay += measFetcher->cycleInterval * 4;
  measBufferBackupStorage->usDelay += measFetcher->cycleInterval * 2;
  overseerArray->usDelay += measFetcher->cycleInterval * 2;
  tcpServer->usDelay += measFetcher->cycleInterval * 2;
  spy->usDelay += measFetcher->cycleInterval * 2;
}

unsigned char HomeIO::start() {
  copyInternalDelays();
  
  const char NUM_THREADS = 6;
  pthread_t threads[NUM_THREADS];
  int rc, i;
  
  HomeIO *h = this;
 
  rc = pthread_create(&threads[0], NULL, ioServerThread,  (void *) h);
  
  // wait for IoServer
  
  while (ioServer->ready == false) {
    usleep(50000);
  }
  
  rc = pthread_create(&threads[1], NULL, measStartThread, (void *) h);
  rc = pthread_create(&threads[2], NULL, tcpServerThread, (void *) h);
  rc = pthread_create(&threads[3], NULL, ioOverseerThread, (void *) h);
  rc = pthread_create(&threads[4], NULL, fileStorageThread, (void *) h);
  rc = pthread_create(&threads[5], NULL, fileBufferBackupThread, (void *) h);
  rc = pthread_create(&threads[6], NULL, spyThread, (void *) h);
 
   // wait for each thread to complete
   for (i=0; i<NUM_THREADS; ++i) {
      // block until thread i completes
      rc = pthread_join(threads[i], NULL);
      logError("In main: thread " + to_string(i) + " is complete");
   }
 
   logError("In main: All threads completed successfully");
   exit(EXIT_SUCCESS);
}
