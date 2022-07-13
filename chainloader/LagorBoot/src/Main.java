

import java.io.File;
import java.io.FileInputStream;
import java.util.function.Consumer;

import jssc.SerialPort;
import jssc.SerialPortException;

public class Main {

    public static SerialPort connectAndSend(SerialPort serialPort, String fileName, Consumer<String> print) throws Exception {
        try {
            if (!serialPort.isOpened()) {
                // Open serial port
                serialPort.openPort();
                // Set params. Also you can set params by this string:
                // serialPort.setParams(9600, 8, 1, 0);
                serialPort.setParams(SerialPort.BAUDRATE_115200, SerialPort.DATABITS_8, SerialPort.STOPBITS_1,
                        SerialPort.PARITY_NONE);
            } else {
                serialPort.removeEventListener();
            }
            print.accept("Connecting to the Raspbootin...");
            // Read 8+3 bytes from the serial port (name + load_ready)
            byte[] buffer;
            String s = "";
            do {
                buffer = serialPort.readBytes(1);
                if (buffer[0] != '\r' && buffer[0] != '\n' && buffer[0] != 3) {
                    s += new String(buffer);
                }
            } while (buffer[0] != 3);
            buffer = new byte[2];
            // get two more 0x03 bytes
            buffer = serialPort.readBytes(2);
            print.accept("\nRaspbootin ready: [" + s + "]");

            if (buffer[0] == 3 && buffer[1] == 3) {
                print.accept("\nReady to send kernel file " + fileName);
                // ready to send file
                File f = new File(fileName);

                if (f.exists() && f.isFile()) {

                    long size = f.length();

                    serialPort.writeByte((byte) (size & 255));
                    size >>= 8;
                    serialPort.writeByte((byte) (size & 255));
                    size >>= 8;
                    serialPort.writeByte((byte) (size & 255));
                    size >>= 8;
                    serialPort.writeByte((byte) (size & 255));

                    // Read 2 bytes from the serial port (file size acknowledge or error)
                    buffer = serialPort.readBytes(2);
                    print.accept("\nGot the response from Raspbootin64: " + new String(buffer));

                    if (buffer[0] == 'O' && buffer[1] == 'K') {
                        // File length OK, proceed with upload.
                        FileInputStream in = new FileInputStream(f);
                        buffer = new byte[1024];
                        int read, total = 0;
                        while ((read = in.read(buffer)) != -1) {
                            for (int i = 0; i < read; i++) {
                                serialPort.writeByte(buffer[i]);
                            }
                            total += read;
                        }
                        in.close();
                        print.accept(
                                "\nSent " + total + " bytes.\n");
                        Thread.sleep(50);
                        int bytesLeft = serialPort.getInputBufferBytesCount();
                        if (bytesLeft > 0) {
                            buffer = serialPort.readBytes(bytesLeft);
                            print.accept(new String(buffer));
                        }

                        while (true) {

                            print.accept(new String(serialPort.readBytes(1)));

                        }
                    } else {
                        print.accept("\nFile size error from server: [" + new String(buffer) + "]");
                    }
                } else {
                    print.accept("\nFile " + fileName + " does not exist, or is a folder!");
                }
            } else {
                print.accept("\nGot wrong bytes from Raspbootin64: [" + new String(buffer) + "]");
            }
        } catch (SerialPortException ex) {
            System.out.println(ex);
        }
        return serialPort;
    }

    public static void main(String[] args) {
        try {
            if (args.length == 2) {
                Main.connectAndSend(new SerialPort(args[0]), args[1], System.out::print).closePort();
            } else if (args.length == 0 || args.length > 2) {
                System.err.println("Usage: java Raspbootin64Client <com_port> <file_path>");
                System.err.println("Example: java Raspbootin64Client COM3 C:\\Temp\\kernel8.img");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

    }

}


