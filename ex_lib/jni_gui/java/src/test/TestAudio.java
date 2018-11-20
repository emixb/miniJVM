package test;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import org.mini.media.AudioDecoder;
import org.mini.media.AudioDevice;
import org.mini.media.AudioFrameListener;
import org.mini.reflect.DirectMemObj;
import org.mini.zip.Zip;

class TestAudio {

    public static byte[] readFile(String s) {
        try {
            File f = new File(s);
            byte[] b = new byte[(int) f.length()];

            FileInputStream dis = new FileInputStream(f);
            dis.read(b);
            dis.close();
            return b;
        } catch (IOException ex) {
            ex.printStackTrace();
        }
        return null;
    }

    static void t1() {
        byte[] b = readFile("./bibi.flac");
        System.out.println("b.len" + b.length);
//        AudioDecoder decoder = new AudioDecoder("./bibi.flac");
        AudioDecoder decoder = new AudioDecoder(b);
        AudioDevice device = new AudioDevice();
        device.config(AudioDevice.mal_format_s16, 2, 22050, null);
        device.init(AudioDevice.mal_device_type_playback, decoder);
        device.start();
        try {
            Thread.sleep(2000);
        } catch (InterruptedException ex) {
        }
        System.out.println("status:" + device.isStarted());
        device.stop();
    }

    static void t2() {

        int format = AudioDevice.mal_format_s16;
        int channels = 2;
        int ratio = 22050;

        final ByteArrayOutputStream baos = new ByteArrayOutputStream();

        final AudioDevice capDevice = new AudioDevice();
        capDevice.config(format, channels, ratio, new AudioFrameListener() {
            @Override
            public void onReceiveFrames(AudioDevice pDevice, int frameCount, DirectMemObj dmo) {
                int readNeed = frameCount * pDevice.channels;
                int bytes = readNeed * AudioDevice.getFormatBytes(capDevice.format);
                byte[] b = new byte[bytes];
                dmo.copyTo(b);
                byte[] c = Zip.compress0(b);
                //System.out.println("b.len=" + b.length + "   , c.len=" + c.length);
                //printArr(b, 0);
                try {
                    baos.write(b);
                    //System.out.println("received bytes: " + b.length);
                } catch (IOException ex) {
                }
            }

            @Override
            public int onSendFrames(AudioDevice pDevice, int frameCount, DirectMemObj dmo) {
                return 0;
            }

            @Override
            public void onStop(AudioDevice pDevice) {
            }
        });
        System.out.println("begin capture ");
        capDevice.init(AudioDevice.mal_device_type_capture, null);

        capDevice.start();
        try {
            Thread.sleep(3 * 1000);
        } catch (InterruptedException ex) {
        }
        capDevice.stop();

        final AudioDevice playDevice = new AudioDevice();
        playDevice.config(format, channels, ratio, new AudioFrameListener() {
            byte[] b = baos.toByteArray();
            int pos = 0;

            {
                System.out.println("total len:" + b.length + "   compressed len:" + Zip.compress0(b).length);
            }

            @Override
            public void onReceiveFrames(AudioDevice pDevice, int frameCount, DirectMemObj dmo) {
            }

            @Override
            public int onSendFrames(AudioDevice pDevice, int frameCount, DirectMemObj dmo) {
                int bytes = dmo.getLength();
                if (pos + bytes > b.length) {
                    bytes = b.length - pos;
                }
                //System.out.println("sent bytes: " + bytes + " pos: " + pos);
                dmo.copyFrom(pos, b, 0, bytes);
                //printArr(b, pos);
                pos += bytes;
                return bytes;
            }

            @Override
            public void onStop(AudioDevice pDevice) {
            }
        });
        playDevice.init(AudioDevice.mal_device_type_playback, null);

        playDevice.start();
        try {
            Thread.sleep(3000);
        } catch (InterruptedException ex) {
        }
        playDevice.stop();

    }

    static void printArr(byte[] b, int pos) {
        for (int i = 0; i < 20 && pos + i < b.length; i++) {
            System.out.print((char) (65 + (b[pos + i] % 26)));
        }
        System.out.println();
    }

    public static void main(String[] args) {
        t1();
//        t2();
    }

}
