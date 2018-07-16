import java.util.Random;
import java.util.concurrent.Semaphore;

public class Lab4
{
  // Configuration
  final static int PORT0 = 0;
  final static int PORT1 = 1;
  final static int MAXLOAD = 5;

  public static void main(String args[]) {
     int NUM_CARS = 10;
     int NUM_AMBS = 1;
     int NUM_CROSSING = 10;
    int i;
    
    System.out.println ("num args: " + args.length);
    if (args.length > 0) {
      try {
        NUM_CROSSING = Integer.parseInt(args[0]);
      } catch (NumberFormatException e) {}
    }
    if (args.length > 1) {
      try {
        NUM_CARS = Integer.parseInt(args[1]);
      } catch (NumberFormatException e) {}
    }
    if (args.length > 2) {
      try {
        NUM_AMBS = Integer.parseInt(args[2]);
      } catch (NumberFormatException e) {}
    }

    Ferry ferry = new Ferry(PORT0,NUM_CROSSING);

    Auto [] automobile = new Auto[NUM_CARS];
    for (i=0; i< 7; i++) automobile[i] = new Auto(i,PORT0,ferry);
    for ( ; i<NUM_CARS ; i++) automobile[i] = new Auto(i,PORT1,ferry);

    // Ambulance ambulance = new Ambulance(PORT0,ferry);
    Ambulance [] ambulance = new Ambulance[NUM_AMBS];
    for (i=0; i< NUM_AMBS; i++) ambulance[i] = new Ambulance(i, (PORT0+i) % 2,ferry);

    /* Start the threads */
    ferry.start();   // Start the ferry thread.
    for (i=0; i<NUM_CARS; i++) automobile[i].start();  // Start automobile threads
    for (i=0; i<NUM_AMBS; i++) ambulance[i].start();  // Start the ambulance thread.

    try {ferry.join();} catch(InterruptedException e) { }; // Wait until ferry terminates.
    System.out.println("Ferry stopped.");
    // Stop other threads.
    for (i=0; i<NUM_CARS; i++) automobile[i].interrupt(); // Let's stop the auto threads.
    for (i=0; i<NUM_AMBS; i++) ambulance[i].interrupt(); // Stop the ambulance thread.
  }
}


class Auto extends Thread { // Class for the auto threads.

  private int id_auto;
  private int port;
  private Ferry fry;

  public Auto(int id, int prt, Ferry ferry)
  {
    this.id_auto = id;
    this.port = prt;
    this.fry = ferry;
  }

  public void run() {

    while (true) {
	  // Terminate
      if(isInterrupted()) break;
	  
      // Delay
      try {sleep((int) (300*Math.random()));} catch (Exception e) { break;}
      System.out.println("Auto " + id_auto + " arrives at port " + port);
  
      // Board
	  Semaphore loading;
	  if(port==0){ 
        loading = fry.port0[0];
	  } else {
        loading= fry.port1[0];
	  }
	  //loading.acquireUninterruptibly();
	  try{
		  loading.acquire();
	  } catch (InterruptedException ex){
		  break;
	  }
      System.out.println("Auto " + id_auto + " boards on the ferry at port " + port);
      if (fry.getPort() != port) System.out.println ("<<<<<<<<<< error loading at wrong port >>>>>>>>");
      fry.addLoad();  // increment the ferry load
      
      // Arrive at the next port
      port = 1 - port ;   
      
	  
	  Semaphore unloading;
	  if(port==0){ 
        unloading = fry.port0[1];
	  } else {
        unloading= fry.port1[1];
	  }
	  //unloading.acquireUninterruptibly();
	  try{
		  unloading.acquire();
	  } catch (InterruptedException ex){
		  break;
	  }
      // disembark    
      System.out.println("Auto " + id_auto + " disembarks from ferry at port " + port);
      if (fry.getPort() != port) System.out.println ("<<<<<<<<<< error unloading at wrong port >>>>>>>>");
      fry.reduceLoad();   // Reduce load
  
      // Terminate
      if(isInterrupted()) break;
    }
    System.out.println("Auto "+id_auto+" terminated");
  }
}

class Ambulance extends Thread { // the Class for the Ambulance thread

  private int id;
  private int port;
  private Ferry fry;

  public Ambulance(int id, int prt, Ferry ferry)
  {
    this.port = prt;
    this.fry = ferry;
    this.id = id;
  }

  public void run() {
     while (true) {	 
	  // Terminate
      if(isInterrupted()) break;
	  
      // Attente
      try {sleep((int) (1000*Math.random()));} catch (Exception e) { break;}
      System.out.println("Ambulance " + id + " arrives at port " + port);
  
      // Board
	  Semaphore loading;
	  if(port==0){ 
        loading = fry.port0[0];
	  } else {
        loading= fry.port1[0];
	  }
	  //loading.acquireUninterruptibly();
	  try{
		  loading.acquire();
	  } catch (InterruptedException ex){
		  break;
	  }
	  
	  
      System.out.println("Ambulance " + id + " boards the ferry at port " + port);
      if (fry.getPort() != port) System.out.println ("<<<<<<<<<< error loading at wrong port >>>>>>>>");
      fry.loadAmbulance();  // increment the load  
      
	  
	  
      // Arrive at the next port
      port = 1 - port ;   
      
	  
	     
	  Semaphore unloading;
	  if(port==0){ 
        unloading = fry.port0[1];
	  } else {
        unloading= fry.port1[1];
	  }
	  //unloading.acquireUninterruptibly();
	  try{
		  unloading.acquire();
	  } catch (InterruptedException ex){
		  break;
	  }
	  //disembark
      System.out.println("Ambulance " + id + " disembarks the ferry at port " + port);
      if (fry.getPort() != port) System.out.println ("<<<<<<<<<< error unloading at wrong port >>>>>>>>");
      fry.unloadAmbulance();   // Reduce load
  
      // Terminate
      if(isInterrupted()) break;
    }
    System.out.println("Ambulance " + id + " terminated.");
  }
}

class Ferry extends Thread // The ferry Class
{
  final static int MAXLOAD = 5;
  private int port=0;  // Start at port 0
  private int load=0;  // Load is zero
  private int numCrossings;  // number of crossings to execute
  private boolean ambulance_loaded = false;
  // Semaphores
  Semaphore loadingDone = new Semaphore(0);
  Semaphore unloadingDone = new Semaphore(0);

  //port Semaphores: [0] loading , [1] unloading
  //port 0
  Semaphore[] port0 = {new Semaphore(0,true),new Semaphore(0)};
  //port 1
  Semaphore[] port1 = {new Semaphore(0,true),new Semaphore(0)};
  
  

  public Ferry(int prt, int nbtours)
  {
    this.port = prt;
    numCrossings = nbtours;
  }

  public void run() {
    System.out.println("Start at port " + port + " with a load of " + load + " vehicles");

    // numCrossings crossings in our day
    for(int i=0 ; i < numCrossings ; i++) {
      // The crossing
	  Semaphore loading;
	  if(port==0){ 
        loading = port0[0];
	  } else {
        loading=port1[0];
	  }
	  loading.release();
	  loadingDone.acquireUninterruptibly();
	  //last vehicle will not signal loading of port, thus blocking all next threads, and it will signal loadingDone
      System.out.println("Departure from port " + port + " with a load of " + load + " vehicles");
      System.out.println("Crossing " + i + " with a load of " + load + " vehicles");
      if (ambulance_loaded) {
        if (load == 0) System.out.println("<<<<<<<<<< error ferry leaving with less load! >>>>>>>>");
      } 
      else {
        if (load != MAXLOAD) System.out.println("<<<<<<<<<< error ferry leaving with less load! >>>>>>>>");
      }
      port = 1 - port;
      try {sleep((int) (100*Math.random()));} catch (Exception e) { }
      // Arrive at port
	  // Disembarkment et loading
	  Semaphore unloading;
	  if(port==0){ 
        unloading = port0[1];
	  } else {
        unloading=port1[1];
	  }
      System.out.println("Arrive at port " + port + " with a load of " + load + " vehicles");
	  
	  unloading.release();
	  unloadingDone.acquireUninterruptibly();
	  //last vehicle to disembark will not signal unloading and it will signal unloadingDone for the ferry
    }
  }

  // methodes to manipulate the load of the ferry
  public int getLoad() { return(load); }
  public int getPort() { return(port); }
  
  //might need to add a lock object instead of synchronziing the methods.
  public synchronized void addLoad() {
    if (load >= MAXLOAD) System.out.println ("<<<<<<<<<< error loadig in a full Ferry! >>>>>>>>");
    load = load + 1; 
    System.out.println ("added load, now " + load);
	if(load==MAXLOAD || ambulance_loaded){
	  loadingDone.release();
	} else {
      Semaphore loading;
	  if(port==0){ 
        loading = port0[0];
	  } else {
        loading= port1[0];
	  }
	  loading.release();
	}
  }
  public synchronized void reduceLoad()  { 
    if (load <= 0) System.out.println ("<<<<<<<<<< error unloading an empty Ferry! >>>>>>>>");
    load = load - 1 ; 
    System.out.println ("removed load, now " + load);
	if(load==0){
	  unloadingDone.release();
	} else {
      Semaphore unloading;
	  if(port==0){ 
        unloading = port0[1];
	  } else {
        unloading= port1[1];
	  }
	  unloading.release();
	}
  }
  public synchronized void loadAmbulance() {
    ambulance_loaded = true;
    addLoad();
  }
  public synchronized void unloadAmbulance(){
    ambulance_loaded = false;
    reduceLoad();
  }
}
