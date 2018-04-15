/**
 * Copyright (c) 2007, DigitalPersona, Inc.
 *
 * File: PropertySheet.java
 *
 * Contents: This class used to get state of the properties
 *
 * Remarks:
 */
package com.digitalpersona.javapos.sampleapp.biometrics;

import jpos.BiometricsControl113;
import jpos.JposException;

import java.lang.reflect.Method;

public class PropertySheet implements Runnable {

    /** The biometrics control. */
    private BiometricsControl113 biometricsControl;
    private ImagePanel m_imagePanel = null;
    
    /** The properties. */
    private String[][] properties = {{"CapCompareFirmwareVersion", ""},
            {"CapPowerReporting", ""}, {"CapPrematchData", ""},
            {"CapRawSensorData", ""}, {"CapRealTimeData", ""},
            {"CapSensorColor", ""}, {"CapSensorOrientation", ""},
            {"CapSensorType", ""}, {"CapStatisticsReporting", ""},
            {"CapTemplateAdaptation", ""}, {"CapUpdateFirmware", ""},
            {"CapUpdateStatistics", ""}, {"Algorithm", ""},
            {"AlgorithmList", ""}, {"AutoDisable", ""},
            {"DataCount", ""}, {"DataEventEnabled", ""},
            {"PowerNotify", ""}, {"PowerState", ""},
            {"RawSensorData", ""}, {"RealTimeDataEnabled", ""},
            {"SensorBPP", ""}, {"SensorColor", ""}, {"SensorHeight", ""},
            {"SensorOrientation", ""}, {"SensorType", ""},
            {"SensorWidth", ""}, {"FreezeEvents", ""},
            {"DeviceServiceVersion", ""}, {"DeviceServiceDescription", ""},
            {"DeviceControlDescription", ""}, {"DeviceControlVersion", ""},
            {"Claimed", ""}, {"PhysicalDeviceDescription", ""},
            {"PhysicalDeviceName", ""}, {"DeviceEnabled", ""}};

    /**
     * Instantiates a new property sheet.
     *
     * @param biometricsControl the biometrics control
     */
    public PropertySheet(BiometricsControl113 biometricsControl, ImagePanel imagePanel) {
        this.biometricsControl = biometricsControl;
        this.m_imagePanel = imagePanel;
        Thread propThread = new Thread(this);
        propThread.setName("Property Collector");
        propThread.start();
    }

    /**
     * Gets the properties.
     *
     * @return the properties
     */
    public String[][] getProperties() {
        return properties;
    }

    /** @see java.lang.Runnable#run() */
    public void run() {
        while (true) {
            if (biometricsControl != null) {
                collectProperties();
                try
				{
                	if (biometricsControl.getRawSensorData() != null)
                		m_imagePanel.showImage(biometricsControl.getRawSensorData(), biometricsControl.getSensorWidth(), biometricsControl.getSensorHeight());
				} catch (JposException e)
				{
				}
            }
            try {
                Thread.sleep(1000);
            } catch (InterruptedException _ex) {
                break;
            }
        }
    }

    /** Collect properties. */
    @SuppressWarnings({"unchecked", "rawtypes"})
    private synchronized void collectProperties() {
        Class biometricsControlClass = biometricsControl.getClass();
        String propertyName = null;
        String value;
        for (int i = 0; i < properties.length; i++) {
            try {
                propertyName = properties[i][0];
                Method method = biometricsControlClass.getMethod(MSG_GET + propertyName, (Class[]) null);
                method.setAccessible(true);
                Object object = method.invoke(biometricsControl, (Object[]) null);
                if (object != null) {
                    value = object.toString();
                } else {
                    value = MSG_UNKNOWN;
                }
            } catch (java.lang.reflect.InvocationTargetException _ex) {
                value = _ex.getCause().getMessage();
            } catch (Throwable _tw) {
                value = _tw.getMessage();
            }

            // Exception for raw sensor data, show message instead of garbage.            
            if (propertyName == "RawSensorData" && value != "Control not opened" && value != "Unknown")
            {  
            	properties[i][1] = "Data Received"; 
            }
            else
            	properties[i][1] = value;  
        }
    }

    public synchronized void setBiometricsControl(BiometricsControl113 biometricsControl) {
        this.biometricsControl = biometricsControl;
        collectProperties();
    }

    /** Messages constant. */
    private static final String MSG_GET = "get";
    private static final String MSG_UNKNOWN = "Unknown";
}