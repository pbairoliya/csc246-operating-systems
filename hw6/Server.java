import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Scanner;
import java.util.Random;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;
import java.security.PublicKey;
import java.security.KeyFactory;
import java.security.spec.X509EncodedKeySpec;
import java.security.GeneralSecurityException;
import java.util.Base64;

/**
 * A server that keeps up with a public key for every user, scrabble
 * word scores for all users.
 */
public class Server {
  volatile int r;
  volatile int c;
  volatile static char board[][];

  volatile Object lock = new Object();
  /** Port number used by the server */
  public static final int PORT_NUMBER = 26306;

  /** Maximum length of a username. */
  public static int NAME_MAX = 10;

  /** Record for an individual user. */
  private static class UserRec {
    // Name of this user.
    String name;

    // This user's public key.
    PublicKey publicKey;
  }

  /** List of all the user records. */
  private ArrayList<UserRec> userList = new ArrayList<UserRec>();

  private static void usage() {
    System.err.println("usage: Server <rows> <cols>");
    System.exit(1);
  }

  /** Read the list of all users and their public keys. */
  private void readUsers() throws Exception {
    Scanner input = new Scanner(new File("passwd.txt"));
    while (input.hasNext()) {
      // Create a record for the next user.
      UserRec rec = new UserRec();
      rec.name = input.next();

      // Get the key as a string of hex digits and turn it into a byte array.
      String base64Key = input.nextLine().trim();
      byte[] rawKey = Base64.getDecoder().decode(base64Key);

      // Make a key specification based on this key.
      X509EncodedKeySpec pubKeySpec = new X509EncodedKeySpec(rawKey);

      // Make an RSA key based on this specification
      KeyFactory keyFactory = KeyFactory.getInstance("RSA");
      rec.publicKey = keyFactory.generatePublic(pubKeySpec);

      // Add this user to the list of all users.
      userList.add(rec);
    }
  }

  /**
   * Utility function to read a length then a byte array from the
   * given stream. TCP doesn't respect message boundaraies, but this
   * is essientially a technique for marking the start and end of
   * each message in the byte stream. As a public, static method,
   * this can also be used by the client.
   */
  public static byte[] getMessage(DataInputStream input) throws IOException {
    int len = input.readInt();
    byte[] msg = new byte[len];
    input.readFully(msg);
    return msg;
  }

  /** Function analogous to the previous one, for sending messages. */
  public static void putMessage(DataOutputStream output, byte[] msg) throws IOException {
    // Write the length of the given message, followed by its contents.
    output.writeInt(msg.length);
    output.write(msg, 0, msg.length);
    output.flush();
  }

  public void fillboard() {

    for (int i = 0; i < r; i++) {
      for (int j = 0; j < c; j++) {
        board[i][j] = ' ';
      }
    }
  }

  /** Handle interaction with our client. */
  public void handleClient(Socket sock) {
    try {
      // Get formatted input/output streams for this thread. These can read and write
      // strings, arrays of bytes, ints, lots of things.
      DataOutputStream output = new DataOutputStream(sock.getOutputStream());
      DataInputStream input = new DataInputStream(sock.getInputStream());

      // Get the username.
      String username = input.readUTF();

      // Make a random sequence of bytes to use as a challenge string.
      Random rand = new Random();
      byte[] challenge = new byte[16];
      rand.nextBytes(challenge);

      // Make a session key for communiating over AES. We use it later, if the
      // client successfully authenticates. Is this a safe way to generate
      // a random key? Someone should check on that.
      byte[] sessionKey = new byte[16];
      rand.nextBytes(sessionKey);

      // Find this user. We don't need to synchronize here, since the set of users
      // never
      // changes.
      UserRec rec = null;
      for (int i = 0; rec == null && i < userList.size(); i++)
        if (userList.get(i).name.equals(username))
          rec = userList.get(i);

      // Did we find a record for this user?
      if (rec != null) {
        // Make sure the client encrypted the challenge properly.
        Cipher RSADecrypter = Cipher.getInstance("RSA");
        RSADecrypter.init(Cipher.DECRYPT_MODE, rec.publicKey);

        Cipher RSAEncrypter = Cipher.getInstance("RSA");
        RSAEncrypter.init(Cipher.ENCRYPT_MODE, rec.publicKey);

        // Send the client the challenge.
        putMessage(output, challenge);

        // Get back the client's encrypted challenge.
        byte[] encryptedChallenge = getMessage(input);

        // Make sure the client properly encrypted the challenge.
        try {
          byte[] decryptedChallege = RSADecrypter.doFinal(encryptedChallenge);

          boolean allow = Arrays.equals(decryptedChallege, challenge);
          if (!allow) {
            return;
          }

        } catch (Exception e) {
          throw e;
        }

        // Send the client our session key (encrypted)
        byte[] encryptedSessionKey = RSAEncrypter.doFinal(sessionKey);
        putMessage(output, encryptedSessionKey);

        // Make AES cipher objects to encrypt and decrypt with
        // the session key.

        SecretKey symmetricKey = new SecretKeySpec(sessionKey, "AES");

        Cipher AESEncrypter = Cipher.getInstance("AES/ECB/PKCS5Padding");
        AESEncrypter.init(Cipher.ENCRYPT_MODE, symmetricKey);

        Cipher AESDecrypter = Cipher.getInstance("AES/ECB/PKCS5Padding");
        AESDecrypter.init(Cipher.DECRYPT_MODE, symmetricKey);

        // Get the first client command
        String request = new String(AESDecrypter.doFinal(getMessage(input)));

        // All requests start with a verb.
        while (!request.equals("quit")) {
          StringBuilder reply = new StringBuilder();

          if (request.equals("board")) {
            reply.append(viewBoard());
          } else {
            String command;
            int startR;
            int startC;
            String word;
            Scanner scanner = new Scanner(request);
            if (!scanner.hasNext()) {
              reply.append("Invalid Command\n");

            } else {
              command = scanner.next();
              if (!scanner.hasNextInt()) {
                reply.append("Invalid Command\n");

              } else {
                startR = scanner.nextInt();
                if (!scanner.hasNextInt()) {
                  reply.append("Invalid Command\n");

                } else {
                  startC = scanner.nextInt();
                  if (!scanner.hasNext()) {
                    reply.append("Invalid Command\n");

                  } else {
                    if (startR < 0 || startC < 0 || startR >= r || startC >= c) {
                      reply.append("Invalid Command\n");
                    } else {
                      word = scanner.next();
                      if (scanner.hasNext()) {
                        reply.append("Invalid Command\n");

                      }
                      char[] wordChars = word.toCharArray();
                      boolean isWord = true;
                      for (char c : wordChars) {
                        if (!Character.isLowerCase(c)) {
                          isWord = false;
                        }
                      }
                      if (!isWord) {
                        reply.append("Invalid Command\n");
                      } else {
                        if (command.equals("across")) {
                          if (!putAcross(startR, startC, word)) {
                            reply.append("Invalid Command\n");
                          }
                        } else if (command.equals("down")) {
                          if (!putDown(startR, startC, word)) {
                            reply.append("Invalid Command\n");
                          }

                        } else {
                          reply.append("Invalid Command\n");
                        }
                      }

                    }
                  }

                }
              }
            }
            scanner.close();
          }
          putMessage(output, AESEncrypter.doFinal(reply.toString().getBytes()));
          // Get the next command.
          request = new String(AESDecrypter.doFinal(getMessage(input)));
        }
      }
    } catch (IOException e) {
      System.out.println("IO Error: " + e);
    } catch (GeneralSecurityException e) {
      System.err.println("Encryption error: " + e);
    } finally {
      try {
        // Close the socket on the way out.
        sock.close();
      } catch (Exception e) {
      }
    }
  }

  public class MyThread implements Runnable {
    Socket threadSocket;

    public MyThread(Socket socket) {
      threadSocket = socket;
    }

    public void run() {
      handleClient(threadSocket);
    }
  }

  /**
   * Esentially, the main method for our server, as an instance method
   * so we can access non-static fields.
   */
  private void run(String[] args) {

    if (args.length != 2) {
      usage();
    }
    r = Integer.parseInt(args[0]);
    c = Integer.parseInt(args[1]);

    if (r <= 0 || c <= 0) {
      usage();
    }
    board = new char[r][c];
    fillboard();

    ServerSocket serverSocket = null;

    // One-time setup.
    try {
      // Read the map and the public keys for all the users.
      readUsers();

      // Open a socket for listening.
      serverSocket = new ServerSocket(PORT_NUMBER);
    } catch (Exception e) {
      System.err.println("Can't initialize server: " + e);
      e.printStackTrace();
      System.exit(1);
    }

    // Keep trying to accept new connections and serve them.
    while (true) {
      try {
        // Try to get a new client connection.
        Socket sock = serverSocket.accept();

        // Handle interaction with this client.
        // MultiThreading
        Thread socketThread = new Thread(new MyThread(sock));

        socketThread.start();
      } catch (IOException e) {
        System.err.println("Failure accepting client " + e);
      }
    }
  }
  /**
   * returns the board and prints it out and prints out the board into a single BoardsString
   * @return String
   */
  public String viewBoard() {
    synchronized (lock) {

      StringBuilder boardString = new StringBuilder();

      boardString.append("+");

      for (int i = 0; i < c; i++) {
        boardString.append("-");
      }
      boardString.append("+\n");
      for (int i = 0; i < r; i++) {
        boardString.append("|");
        for (int j = 0; j < c; j++) {
          boardString.append(board[i][j]);

        }
        boardString.append("|\n");
      }
      boardString.append("+");
      for (int i = 0; i < c; i++) {
        boardString.append("-");
      }
      boardString.append("+\n");

      return boardString.toString();

    }

  }
  /**
   * puts a word on the global board put across
   * @param startR the row position to start off with 
   * @param startC the column positioin to start off with 
   * @param word the word you are tryint to put on
   * @return the success boolean value true or false if it was succesful
   */
  public boolean putAcross(int startR, int startC, String word) {
    synchronized (lock) {

      int length = word.length();
      if ((startC + length) > c) {

        return false;
      }
      int count = 0;
      for (int i = startC; i < startC + length; i++) {
        if (board[startR][i] != ' ') {
          if (board[startR][i] != word.charAt(count)) {

            return false;
          }
        }
        count++;
      }
      count = 0;
      for (int i = startC; i < startC + length; i++) {
        board[startR][i] = word.charAt(count++);
      }
      return true;
    }

  }
  /**
   * puts a word on the global board downwards
   * @param startR the row position to start off with 
   * @param startC the column positioin to start off with 
   * @param word the word you are tryint to put on
   * @return the success boolean value true or false if it was succesful
   */
  public boolean putDown(int startR, int startC, String word) {
    synchronized (lock) {

      int length = word.length();
      if ((startR + length) > r) {

        return false;
      }
      int count = 0;
      for (int i = startR; i < startR + length; i++) {
        if (board[i][startC] != ' ') {
          if (board[i][startC] != word.charAt(count)) {

            return false;
          }
        }
        count++;
      }
      count = 0;
      for (int i = startR; i < startR + length; i++) {
        board[i][startC] = word.charAt(count++);
      }

      return true;
    }
  }

  // Main method that runs the server
  public static void main(String[] args) {
    // Make a server object, so we can use non-static fields and methods.
    Server server = new Server();
    server.run(args);
  }
}
