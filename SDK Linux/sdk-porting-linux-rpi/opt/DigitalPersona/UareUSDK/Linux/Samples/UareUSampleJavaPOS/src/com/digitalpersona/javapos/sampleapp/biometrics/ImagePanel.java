package com.digitalpersona.javapos.sampleapp.biometrics;
 
import java.awt.Graphics;
import java.awt.image.BufferedImage;

import javax.swing.JPanel;

public class ImagePanel 
	extends JPanel
{
	public ImagePanel() {
        
	}
	
	private static final long serialVersionUID = 5;
	private BufferedImage m_image;
	private int m_width = 0;
	private int m_height = 0;
	
	public void showImage(byte[] image, int width, int height){
		m_image = new BufferedImage(width, height, BufferedImage.TYPE_BYTE_GRAY);
		m_image.getRaster().setDataElements(0, 0, width, height, image);
		
		m_width = width;
		m_height = height;
		
		repaint();
	} 
	
	public void paint(Graphics g) {
		if (m_width != 0 && m_height != 0)
		{
			g.drawImage(m_image, -38, 0, 195, 132, null); // Arbitrary scaling to fit 4500 and 5100 images onto screen.
		}
	}

}
