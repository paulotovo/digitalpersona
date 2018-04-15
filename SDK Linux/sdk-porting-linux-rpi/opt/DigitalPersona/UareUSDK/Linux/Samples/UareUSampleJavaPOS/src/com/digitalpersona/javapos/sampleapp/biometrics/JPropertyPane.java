/**
 * Copyright (c) 2007, DigitalPersona, Inc.
 *
 * File: JPropertyPane.java
 *
 * Contents: Grid implementaion to display properties.
 *
 * Remarks:
 */
package com.digitalpersona.javapos.sampleapp.biometrics;

import jpos.BiometricsControl113;

import javax.swing.*;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.TableModel;

public class JPropertyPane extends JScrollPane {
	private static final long serialVersionUID = 1L;
	/** Holds the reference to the PropertySheet class. */
    private PropertySheet ps = null;
    /** Holds propertySheetColumns information. */
    private final String[] propertySheetColumns = {PROPERTY_NAME, PROPERTY_VALUE};
    /**
     * Description : Constructor (Used to initialize the property pane for the UI Component)
     *
     * @param biometricsControl control which prperties needs to be displayed
     */
    public JPropertyPane(BiometricsControl113 biometricsControl, ImagePanel imagePanel) {
        super(new JTextArea(),
                JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
                JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);

        ps = new PropertySheet(biometricsControl, imagePanel);

        TableModel dataModel = new AbstractTableModel() {
			private static final long serialVersionUID = 1L;

			public int getColumnCount() {
                return propertySheetColumns.length;
            }

            public int getRowCount() {
                return ps.getProperties().length;
            }

            public Object getValueAt(int row, int col) {
                return ps.getProperties()[row][col];
            }

            public String getColumnName(int column) {
                return propertySheetColumns[column];
            }

            @SuppressWarnings({"unchecked", "rawtypes"})
			public Class getColumnClass(int c) {
                return getValueAt(0, c).getClass();
            }

            public boolean isCellEditable(int row, int col) {
                return true;
            }

            public void setValueAt(Object aValue, int row, int column) {
                ps.getProperties()[row][column] = (String) aValue;
            }
        };

        JTable propertyTable = new JTable(dataModel);
        propertyTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
        propertyTable.getColumn(PROPERTY_NAME).setPreferredWidth(168);
        propertyTable.getColumn(PROPERTY_VALUE).setPreferredWidth(235);

        setViewportView(propertyTable);
    }

    /** Column headers */
    private static final String PROPERTY_NAME = "Property Name";
    private static final String PROPERTY_VALUE = "Value";

    public void setBiometricsControl(BiometricsControl113 biometricsControl) {
        if (ps != null)
            ps.setBiometricsControl(biometricsControl);
    }
}
