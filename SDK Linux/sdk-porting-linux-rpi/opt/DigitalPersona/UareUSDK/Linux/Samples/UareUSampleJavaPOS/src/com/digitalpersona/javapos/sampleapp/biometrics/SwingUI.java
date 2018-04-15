package com.digitalpersona.javapos.sampleapp.biometrics;

import com.digitalpersona.javapos.utils.LogOutputStream;
import jpos.BiometricsControl113;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.PrintStream;
import java.util.EnumSet;
import java.util.Enumeration;

/**
 * Swing UI for sample application.
 */
public class SwingUI extends JFrame implements UI {
	private static final long serialVersionUID = 1L;

	private JPropertyPane jPropScrollPane;

    /**
     * Instantiates a new GUI for sample application
     */
    public SwingUI() {
        setDefaultFont();
        InitializeGUI();

        //catch the log written to stderr
        PrintStream ps = new PrintStream(new LogOutputStream(jLogArea));
        System.setErr(ps);
    }

    /**
     * Sets the prompt.
     *
     * @param prompt prompt to set.
     */
    public void displayMessage(final String prompt) {
        final Runnable runnable = new Runnable() {
            public void run() {
                jStatusLabel.setText(prompt);
            }
        };

        if (SwingUtilities.isEventDispatchThread()) {
            runnable.run();
        } else {
            SwingUtilities.invokeLater(runnable);
        }
    }

    public void logMessage(String message) {
        System.err.println(message);
    }

    public void setEnabledOperations(final EnumSet<Operation> operations) {
        Runnable runnable = new Runnable() {
            public void run() {
                jButtonsPanel.setButtons(
                        operations.contains(Operation.OPEN),
                        operations.contains(Operation.CHECK_HEALTH),
                        operations.contains(Operation.CLOSE),
                        operations.contains(Operation.CLAIM),
                        operations.contains(Operation.RELEASE),
                        operations.contains(Operation.DEVICE_ENABLE),
                        operations.contains(Operation.DEVICE_DISABLE),
                        operations.contains(Operation.CLEAR_DATA),
                        operations.contains(Operation.BEGIN_ENROLL_CAPTURE),
                        operations.contains(Operation.END_CAPTURE),
                        operations.contains(Operation.BEGIN_VERIFY_CAPTURE),
                        operations.contains(Operation.IDENTIFY_MATCH),
                        operations.contains(Operation.VERIFY_MATCH),
                        operations.contains(Operation.IDENTIFY),
                        operations.contains(Operation.VERIFY));
            }
        };

        if (SwingUtilities.isEventDispatchThread()) {
            runnable.run();
        } else {
            SwingUtilities.invokeLater(runnable);
        }
    }

    public void addOperationListener(OperationListener listener) {
        this.listener = listener;
    }

    public void removeOperationListener(OperationListener listener) {
        if (this.listener == listener) {
            this.listener = null;
        }
    }

    public void setBiometricsControl(BiometricsControl113 biometricsControl) {
        this.biometricsControl = biometricsControl;
        if (jPropScrollPane != null)
            jPropScrollPane.setBiometricsControl(biometricsControl);
    }

    /**
     * Main init. This method is used to initialize UI components.
     */
    private void InitializeGUI() {
        setSize(new Dimension(823, 652));
        setResizable(false);
        setState(Frame.NORMAL);
        setTitle(TITLE);
        setLayout(null);
        setLocation(10, 10);
        addWindowListener(new WindowAdapter() {
            /**
             * Invoked when a window is in the process of being closed.
             * The close operation can be overridden at this point.
             */
            public void windowClosing(WindowEvent e) {
                this_windowClosing(e);
            }
        });

        jHeadLabel.setFont(new java.awt.Font("", Font.PLAIN, 18));
        jHeadLabel.setText(TITLE);
        jHeadLabel.setForeground(Color.black);
        jHeadLabel.setBounds(new Rectangle(100, 0, 580, 40));
        add(jHeadLabel);

        jStatusLabel.setFont(new java.awt.Font("Verdana", Font.BOLD, 14));
        jStatusLabel.setForeground(Color.blue);
        jStatusLabel.setBounds(new Rectangle(8, 30, 550, 20));
        add(jStatusLabel);

        InitButtonsPanel();
        add(jButtonsPanel);

        jLogArea.setBounds(new Rectangle(180, 180, 410, 300));
        jLogArea.setBackground(Color.WHITE);
        jLogArea.setForeground(Color.BLACK);
        jLogArea.setFont(new java.awt.Font("Verdana", 0, 10));
        jLogArea.setEditable(false);

        JScrollPane jScrollPane = new JScrollPane(jLogArea,
                JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        jScrollPane.setBorder(BorderFactory.createEtchedBorder(Color.GREEN,
                new Color(148, 145, 140)));
        jScrollPane.setBounds(167, 50, 331, 539);
        add(jScrollPane);

        jPropScrollPane = new JPropertyPane(biometricsControl, imagePanel);
        jPropScrollPane.setBounds(497, 50, 310, 539);
        jPropScrollPane.setBorder(BorderFactory.createEtchedBorder(Color.GREEN,
                new Color(148, 145, 140)));
        add(jPropScrollPane);

        jVersionLabel.setBounds(350, 592, 440, 20);
        jVersionLabel.setFont(new java.awt.Font("", Font.BOLD, 11));
        jVersionLabel.setForeground(Color.GRAY);
        jVersionLabel.setText(VERSION);
        add(jVersionLabel);

        setVisible(true);
    }

    private void InitButtonsPanel() {
    	imagePanel.setBounds(10, 457, 158, 132);
    	imagePanel.setLayout(null);
    	imagePanel.setBackground(Color.white);
    	imagePanel.setBorder(BorderFactory.createEtchedBorder(Color.GREEN,
                new Color(148, 145, 140)));
        add(imagePanel);
    	
        jButtonsPanel.setBounds(new Rectangle(10, 50, 158, 407));
        jButtonsPanel.setLayout(null);

        jButtonsPanel.addOpenListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (listener != null)
                    listener.onOperation(new OperationEvent(this, Operation.OPEN));
            }
        });

        jButtonsPanel.addCheckHealthListener (new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (listener != null)
                    listener.onOperation(new OperationEvent(this, Operation.CHECK_HEALTH));
            }
        });

        jButtonsPanel.addCloseListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (listener != null)
                    listener.onOperation(new OperationEvent(this, Operation.CLOSE));
            }
        });

        jButtonsPanel.addClaimListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (listener != null)
                    listener.onOperation(new OperationEvent(this, Operation.CLAIM));
            }
        });

        jButtonsPanel.addReleaseListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (listener != null)
                    listener.onOperation(new OperationEvent(this, Operation.RELEASE));
            }
        });

        jButtonsPanel.addDeviceEnableListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (listener != null)
                    listener.onOperation(new OperationEvent(this, Operation.DEVICE_ENABLE));
            }
        });

        jButtonsPanel.addDeviceDisableListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (listener != null)
                    listener.onOperation(new OperationEvent(this, Operation.DEVICE_DISABLE));
            }
        });

        jButtonsPanel.addClearDataListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (listener != null)
                    listener.onOperation(new OperationEvent(this, Operation.CLEAR_DATA));
            }
        });

        jButtonsPanel.addBeginEnrollCaptureListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (listener != null)
                    listener.onOperation(new OperationEvent(this, Operation.BEGIN_ENROLL_CAPTURE));
            }
        });

        jButtonsPanel.addEndCaptureListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (listener != null)
                    listener.onOperation(new OperationEvent(this, Operation.END_CAPTURE));
            }
        });

        jButtonsPanel.addBeginVerifyCaptureListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (listener != null)
                    listener.onOperation(new OperationEvent(this, Operation.BEGIN_VERIFY_CAPTURE));
            }
        });

        jButtonsPanel.addIdentifyMatchListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (listener != null)
                    listener.onOperation(new OperationEvent(this, Operation.IDENTIFY_MATCH));
            }
        });

        jButtonsPanel.addVerifyMatchListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (listener != null)
                    listener.onOperation(new OperationEvent(this, Operation.VERIFY_MATCH));
            }
        });

        jButtonsPanel.addIdentifyListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (listener != null)
                    listener.onOperation(new OperationEvent(this, Operation.IDENTIFY));
            }
        });

        jButtonsPanel.addVerifyListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (listener != null)
                    listener.onOperation(new OperationEvent(this, Operation.VERIFY));
            }
        });
    }

    /**
     * This method is called when the user closes the window.
     *
     * @param e WindowEvent occured
     * @see java.awt.event.WindowEvent
     */
    public void this_windowClosing(WindowEvent e) {
        try {
            biometricsControl.close();
        } catch (Exception ex) {
        }
        System.exit(0);
    }

    /**
     * Sets the default font.
     */
    private void setDefaultFont() {
        Enumeration<Object> names = UIManager.getDefaults().keys();
        while (names.hasMoreElements()) {
            Object key = names.nextElement();
            Object value = UIManager.get(key);
            if (value instanceof javax.swing.plaf.FontUIResource) {
                UIManager.put(key, new javax.swing.plaf.FontUIResource(
                        "verdana", Font.PLAIN, 10));
            }
        }
    }

    /**
     * The jLogArea is used to dipslay the execution log
     */
    private JTextArea jLogArea = new JTextArea();

    /**
     * The jStatusLabel is used to display the text.
     */
    private JLabel jStatusLabel = new JLabel();

    /**
     * The jHeadLabel is used to display the header text.
     */
    private JLabel jHeadLabel = new JLabel();

    /**
     * The jVersionLabel is used to display the version number.
     */
    private JLabel jVersionLabel = new JLabel();

    /**
     * The jButtonsPanel is used ti hold the buttons.
     */
    private JButtonsPanel jButtonsPanel = new JButtonsPanel();
    private ImagePanel imagePanel = new ImagePanel();
    private OperationListener listener;
    private BiometricsControl113 biometricsControl;

    /**
     * Sample application version.
     */
    private String VERSION = "DigitalPersona Sample Application for JavaPOS Biometrics Device v2.0.0";

    /**
     * Sample application title.
     */
    private static final String TITLE = "DigitalPersona Sample Application for JavaPOS Biometrics Device";

    /**
     * When an object implementing interface <code>Runnable</code> is used
     * to create a thread, starting the thread causes the object's
     * <code>run</code> method to be called in that separately executing
     * thread.
     * <p/>
     * The general contract of the method <code>run</code> is that it may
     * take any action whatsoever.
     *
     * @see Thread#run()
     */
    public void run() {
        while (true) {
            repaint();
            try {
                Thread.sleep(1000);
            } catch (InterruptedException _ex) {
                break;
            }
        }
    }
}
