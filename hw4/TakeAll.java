import java.util.Random;


/**
 * Applies no hold and wait monitor technique to be faster then ordered and global java. 
 */
public class TakeAll {
  /** To tell all the chefs when they can quit running. */
  private static boolean running = true;

  /**
   * Superclass for all chefs. Contains methods to cook and rest and
   * keeps a record of how many dishes were prepared.
   */
  private static class Chef extends Thread {
    /** Number of dishes prepared by this chef. */
    private int dishCount = 0;

    /** Source of randomness for this chef. */
    private Random rand = new Random();

    /**
     * Called after the chef has locked all the required appliances and is
     * ready to cook for about the given number of milliseconds.
     */
    protected void cook(int duration) {
      System.out.printf("%s is cooking\n", getClass().getSimpleName());
      try {
        // Wait for a while (pretend to be cooking)
        Thread.sleep(rand.nextInt(duration / 2) + duration / 2);
      } catch (InterruptedException e) {
      }
      dishCount++;
    }

    /** Called between dishes, to let the chef rest before cooking another dish. */
    protected void rest(int duration) {
      System.out.printf("%s is resting\n", getClass().getSimpleName());
      try {
        // Wait for a while (pretend to be resting)
        Thread.sleep(rand.nextInt(duration / 2) + duration / 2);
      } catch (InterruptedException e) {
      }
    }
  }

  // An object representing the lock on each appliance.
  // Locking the needed objects before cooking prevents two
  // chefs from trying to use the same appliance at the same time.
  private static boolean griddle = true;
  private static boolean mixer = true;
  private static boolean oven = true;
  private static boolean blender = true;
  private static boolean grill = true;
  private static boolean fryer = true;
  private static boolean microwave = true;
  private static boolean coffeeMaker = true;

  private static Object lock = new Object();

  /** Mandy is a chef needing 105 milliseconds to prepare a dish. */
  private static class Mandy extends Chef {
    public void run() {
      while (running) {
        synchronized (lock) {
          while (!coffeeMaker || !microwave ) {
            try {
              lock.wait();
            } catch (InterruptedException e) {
              // TODO Auto-generated catch block
              e.printStackTrace();
            }
          }

          coffeeMaker = false;
          microwave = false;

        }
        cook(105);
        synchronized (lock) {
          coffeeMaker = true;
          microwave = true;
          lock.notifyAll();
        }
        rest(25);
      }
    }
  }

  /** Edmund is a chef needing 30 milliseconds to prepare a dish. */
  private static class Edmund extends Chef {
    public void run() {
      while (running) {
        synchronized (lock) {
          while (!blender || !mixer || !oven) {
            try {
              lock.wait();
            } catch (InterruptedException e) {
              // TODO Auto-generated catch block
              e.printStackTrace();
            }
          }

          blender = false;
          mixer = false;
          oven = false;

        }

        cook(30);

        synchronized (lock) {
          blender = true;
          mixer = true;
          oven = true;
          lock.notifyAll();
        }
        rest(25);
      }
    }
  }

  /** Napoleon is a chef needing 60 milliseconds to prepare a dish. */
  private static class Napoleon extends Chef {
    public void run() {
      while (running) {
        // Get the appliances this chef uses.
        synchronized (lock) {
          while (!blender || !grill) {
            try {
              lock.wait();
            } catch (InterruptedException e) {
              // TODO Auto-generated catch block
              e.printStackTrace();
            }
          }

          blender = false;
          grill = false;
        }

        cook(60);
        synchronized (lock) {
          blender = true;
          grill = true;
          lock.notifyAll();
        }
        rest(25);
      }
    }
  }

  /** Prudence is a chef needing 15 milliseconds to prepare a dish. */
  private static class Prudence extends Chef {
    public void run() {
      while (running) {
        synchronized (lock) {
          while (!coffeeMaker || !griddle || !microwave) {
            try {
              lock.wait();
            } catch (InterruptedException e) {
              // TODO Auto-generated catch block
              e.printStackTrace();
            }
          }

          coffeeMaker = false;
          griddle = false;
          microwave = false;

        }

        cook(15);
        synchronized (lock) {
          coffeeMaker = true;
          griddle = true;
          microwave = true;
          lock.notifyAll();
        }

        rest(25);
      }
    }
  }

  /** Kyle is a chef needing 45 milliseconds to prepare a dish. */
  private static class Kyle extends Chef {
    public void run() {
      while (running) {

        synchronized (lock) {
          while (!fryer || !oven) {
            try {
              lock.wait();
            } catch (InterruptedException e) {
              // TODO Auto-generated catch block
              e.printStackTrace();
            }
          }

          fryer = false;
          oven = false;
        }
        cook(45);
        synchronized (lock) {
          fryer = true;
          oven = true;
          lock.notifyAll();
        }
        rest(25);
      }
    }
  }

  /** Claire is a chef needing 15 milliseconds to prepare a dish. */
  private static class Claire extends Chef {
    public void run() {
      while (running) {


        synchronized (lock) {
          while (!grill || !griddle) {
            try {
              lock.wait();
            } catch (InterruptedException e) {
              // TODO Auto-generated catch block
              e.printStackTrace();
            }
          }

          grill = false;
          griddle = false;
        }
        cook(15);
        synchronized(lock){
          grill = true;
          griddle = true;
          lock.notifyAll();
        }
        rest(25);
      }
    }
  }

  /** Lucia is a chef needing 15 milliseconds to prepare a dish. */
  private static class Lucia extends Chef {
    public void run() {
      while (running) {
       
        synchronized(lock){
          while(!griddle || !mixer){
            try {
              lock.wait();
            } catch (InterruptedException e) {
              // TODO Auto-generated catch block
              e.printStackTrace();
            }
          }

          griddle = false;
          mixer = false;
        }
        cook(15);
        synchronized(lock){
          griddle = true;
          mixer = true;
          lock.notifyAll();
        }
        rest(25);
      }
    }
  }

  /** Marcos is a chef needing 60 milliseconds to prepare a dish. */
  private static class Marcos extends Chef {
    public void run() {
      while (running) {


        synchronized(lock){
          while(!blender || !fryer || !microwave){
            try {
              lock.wait();
            } catch (InterruptedException e) {
              // TODO Auto-generated catch block
              e.printStackTrace();
            }
          }

          blender = false;
          fryer = false;
          microwave = false;
        }
        cook(60);
        synchronized(lock){
          blender = true;
          fryer = true;
          microwave = true;
          lock.notifyAll();
        }
        rest(25);
      }
    }
  }

  /** Roslyn is a chef needing 75 milliseconds to prepare a dish. */
  private static class Roslyn extends Chef {
    public void run() {
      while (running) {

        synchronized(lock){
          while(!fryer || !grill){
            try {
              lock.wait();
            } catch (InterruptedException e) {
              // TODO Auto-generated catch block
              e.printStackTrace();
            }
          }

          fryer = false;
          grill = false;
        }
        cook(75);

        synchronized(lock){
          fryer = true;
          grill = true;
          lock.notifyAll();
        }


        rest(25);
      }
    }
  }

  /** Stephenie is a chef needing 30 milliseconds to prepare a dish. */
  private static class Stephenie extends Chef {
    public void run() {
      while (running) {
        // Get the appliances this chef uses.

        synchronized(lock){
          while(!coffeeMaker || !mixer || !oven){
            try {
              lock.wait();
            } catch (InterruptedException e) {
              // TODO Auto-generated catch block
              e.printStackTrace();
            }
          }

          coffeeMaker = false;
          mixer = false;
          oven = false;
        }
        cook(30);
        synchronized(lock){
          coffeeMaker = true;
          mixer = true;
          oven = true;
          lock.notifyAll();
        }

        rest(25);
      }
    }
  }

  public static void main(String[] args) throws InterruptedException {
    // Make a thread for each of our chefs.
    Chef chefList[] = {
        new Mandy(),
        new Edmund(),
        new Napoleon(),
        new Prudence(),
        new Kyle(),
        new Claire(),
        new Lucia(),
        new Marcos(),
        new Roslyn(),
        new Stephenie(),
    };

    // Start running all our chefs.
    for (int i = 0; i < chefList.length; i++)
      chefList[i].start();

    // Let the chefs cook for a while, then ask them to stop.
    Thread.sleep(10000);
    running = false;

    // Wait for all our chefs to finish, and collect up how much
    // cooking was done.
    int total = 0;
    for (int i = 0; i < chefList.length; i++) {
      chefList[i].join();
      System.out.printf("%s cooked %d dishes\n",
          chefList[i].getClass().getSimpleName(),
          chefList[i].dishCount);
      total += chefList[i].dishCount;
    }
    System.out.printf("Total dishes cooked: %d\n", total);
  }
}
