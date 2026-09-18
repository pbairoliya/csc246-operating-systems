import java.util.Random;

public class Kitchen {
  /** To tell all the chefs when they can quit running. */
  private static boolean running = true;

  /** Superclass for all chefs.  Contains methods to cook and rest and
      keeps a record of how many dishes were prepared. */
  private static class Chef extends Thread {
    /** Number of dishes prepared by this chef. */
    private int dishCount = 0;

    /** Source of randomness for this chef. */
    private Random rand = new Random();

    /** Called after the chef has locked all the required appliances and is
        ready to cook for about the given number of milliseconds. */
    protected void cook( int duration ) {
      System.out.printf( "%s is cooking\n", getClass().getSimpleName() );
      try {
        // Wait for a while (pretend to be cooking)
        Thread.sleep( rand.nextInt( duration / 2 ) + duration / 2 );
      } catch ( InterruptedException e ) {
      }
      dishCount++;
    }

    /** Called between dishes, to let the chef rest before cooking another dish. */
    protected void rest( int duration ) {
      System.out.printf( "%s is resting\n", getClass().getSimpleName() );
      try {
        // Wait for a while (pretend to be resting)
        Thread.sleep( rand.nextInt( duration / 2 ) + duration / 2 );
      } catch ( InterruptedException e ) {
      }
    }
  }

  // An object representing the lock on each appliance.
  // Locking the needed objects before cooking prevents two
  // chefs from trying to use the same appliance at the same time.
  private static Object griddle = new Object();
  private static Object mixer = new Object();
  private static Object oven = new Object();
  private static Object blender = new Object();
  private static Object grill = new Object();
  private static Object fryer = new Object();
  private static Object microwave = new Object();
  private static Object coffeeMaker = new Object();

  /** Mandy is a chef needing 105 milliseconds to prepare a dish. */
  private static class Mandy extends Chef {
    public void run() {
      while ( running ) {
        // Get the appliances this chef uses.
        synchronized ( microwave ) {
          synchronized ( coffeeMaker ) {
            cook( 105 );
          }
        }

        rest( 25 );
      }
    }
  }

  /** Edmund is a chef needing 30 milliseconds to prepare a dish. */
  private static class Edmund extends Chef {
    public void run() {
      while ( running ) {
        // Get the appliances this chef uses.
        synchronized ( blender ) {
          synchronized ( oven ) {
            synchronized ( mixer ) {
              cook( 30 );
            }
          }
        }

        rest( 25 );
      }
    }
  }

  /** Napoleon is a chef needing 60 milliseconds to prepare a dish. */
  private static class Napoleon extends Chef {
    public void run() {
      while ( running ) {
        // Get the appliances this chef uses.
        synchronized ( blender ) {
          synchronized ( grill ) {
            cook( 60 );
          }
        }

        rest( 25 );
      }
    }
  }

  /** Prudence is a chef needing 15 milliseconds to prepare a dish. */
  private static class Prudence extends Chef {
    public void run() {
      while ( running ) {
        // Get the appliances this chef uses.
        synchronized ( coffeeMaker ) {
          synchronized ( microwave ) {
            synchronized ( griddle ) {
              cook( 15 );
            }
          }
        }

        rest( 25 );
      }
    }
  }

  /** Kyle is a chef needing 45 milliseconds to prepare a dish. */
  private static class Kyle extends Chef {
    public void run() {
      while ( running ) {
        // Get the appliances this chef uses.
        synchronized ( fryer ) {
          synchronized ( oven ) {
            cook( 45 );
          }
        }

        rest( 25 );
      }
    }
  }

  /** Claire is a chef needing 15 milliseconds to prepare a dish. */
  private static class Claire extends Chef {
    public void run() {
      while ( running ) {
        // Get the appliances this chef uses.
        synchronized ( grill ) {
          synchronized ( griddle ) {
            cook( 15 );
          }
        }

        rest( 25 );
      }
    }
  }

  /** Lucia is a chef needing 15 milliseconds to prepare a dish. */
  private static class Lucia extends Chef {
    public void run() {
      while ( running ) {
        // Get the appliances this chef uses.
        synchronized ( griddle ) {
          synchronized ( mixer ) {
            cook( 15 );
          }
        }

        rest( 25 );
      }
    }
  }

  /** Marcos is a chef needing 60 milliseconds to prepare a dish. */
  private static class Marcos extends Chef {
    public void run() {
      while ( running ) {
        // Get the appliances this chef uses.
        synchronized ( microwave ) {
          synchronized ( fryer ) {
            synchronized ( blender ) {
              cook( 60 );
            }
          }
        }

        rest( 25 );
      }
    }
  }

  /** Roslyn is a chef needing 75 milliseconds to prepare a dish. */
  private static class Roslyn extends Chef {
    public void run() {
      while ( running ) {
        // Get the appliances this chef uses.
        synchronized ( fryer ) {
          synchronized ( grill ) {
            cook( 75 );
          }
        }

        rest( 25 );
      }
    }
  }

  /** Stephenie is a chef needing 30 milliseconds to prepare a dish. */
  private static class Stephenie extends Chef {
    public void run() {
      while ( running ) {
        // Get the appliances this chef uses.
        synchronized ( mixer ) {
          synchronized ( coffeeMaker ) {
            synchronized ( oven ) {
              cook( 30 );
            }
          }
        }

        rest( 25 );
      }
    }
  }

  public static void main( String[] args ) throws InterruptedException {
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
    for ( int i = 0; i < chefList.length; i++ )
      chefList[ i ].start();

    // Let the chefs cook for a while, then ask them to stop.
    Thread.sleep( 10000 );
    running = false;

    // Wait for all our chefs to finish, and collect up how much
    // cooking was done.
    int total = 0;
    for ( int i = 0; i < chefList.length; i++ ) {
      chefList[ i ].join();
      System.out.printf( "%s cooked %d dishes\n",
                         chefList[ i ].getClass().getSimpleName(),
                         chefList[ i ].dishCount );
      total += chefList[ i ].dishCount;
    }
    System.out.printf( "Total dishes cooked: %d\n", total );
  }
}
