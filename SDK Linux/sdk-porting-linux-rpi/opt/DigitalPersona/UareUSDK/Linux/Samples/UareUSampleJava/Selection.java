import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Vector;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.ListSelectionModel;

import com.digitalpersona.uareu.*;

public class Selection 
	extends JPanel
	implements ActionListener
{
	private static final long serialVersionUID = 2;
	private static final String ACT_BACK = "back";
	private static final String ACT_REFRESH = "refresh";
	private static final String ACT_GETCAPS = "getcaps";
	
	private ReaderCollection m_collection; 
	private JList            m_listReaders;
	private JDialog m_dlgParent;
	
	private Selection(ReaderCollection collection){
		m_collection = collection;
		
		final int vgap = 5;

		BoxLayout layout = new BoxLayout(this, BoxLayout.Y_AXIS);
		setLayout(layout);
		
		add(Box.createVerticalStrut(vgap));
		JLabel lblReader = new JLabel("Available readers:");
		add(lblReader);
		add(Box.createVerticalStrut(vgap));
		
		m_listReaders = new JList();
		m_listReaders.getSelectionModel().setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
		JScrollPane paneReaders = new JScrollPane(m_listReaders);
		add(paneReaders);
		add(Box.createVerticalStrut(vgap));
		
		JButton btnRefresh = new JButton("Refresh list");
		btnRefresh.setActionCommand(ACT_REFRESH);
		btnRefresh.addActionListener(this);
		add(btnRefresh);
		add(Box.createVerticalStrut(vgap));

		JButton btnGetCaps = new JButton("Get reader capabilities");
		btnGetCaps.setActionCommand(ACT_GETCAPS);
		btnGetCaps.addActionListener(this);
		add(btnGetCaps);
		add(Box.createVerticalStrut(vgap));

		JButton btnBack = new JButton("Back");
		btnBack.setActionCommand(ACT_BACK);
		btnBack.addActionListener(this);
		add(btnBack);
		add(Box.createVerticalStrut(vgap));

		setOpaque(true);
	}
	
	private void doModal(JDialog dlgParent){
		RefreshList();
		m_dlgParent = dlgParent;
		m_dlgParent.setContentPane(this);
		m_dlgParent.pack();
		m_dlgParent.setLocationRelativeTo(null);
		m_dlgParent.toFront();
		m_dlgParent.setVisible(true);
		m_dlgParent.dispose();
	}
	
	public void actionPerformed(ActionEvent e){
		if(e.getActionCommand().equals(ACT_BACK)){
			m_dlgParent.setVisible(false);
		}
		if(e.getActionCommand().equals(ACT_REFRESH)){
			RefreshList();
		}
		if(e.getActionCommand().equals(ACT_GETCAPS)){
			Reader reader = getSelectedReader();
			if(null != reader) Capabilities.Show(reader);
		}
	}
	
	private void RefreshList(){
		//acquire available readers
		try{
			m_collection.GetReaders();
		} 
		catch(UareUException e) { 
			MessageBox.DpError("ReaderCollection.GetReaders()", e);
		}
		
		//list reader names
		Vector<String> strNames = new Vector<String>();
		for(int i = 0; i < m_collection.size(); i++){
			strNames.add(m_collection.get(i).GetDescription().name);
		}
		m_listReaders.setListData(strNames);
	}
	
	private Reader getSelectedReader(){
		if(-1 == m_listReaders.getSelectedIndex()) return null;
		return m_collection.get(m_listReaders.getSelectedIndex());
	}
	
	public static Reader Select(ReaderCollection collection){
    	JDialog dlg = new JDialog((JDialog)null, "Select reader", true);
    	Selection selection = new Selection(collection);
    	selection.doModal(dlg);
		return selection.getSelectedReader();
	}
}
