/**
 *  file:   FormChunkExtractView.h
 *  class:  CFormChunkExtractView
 *
 *  Form for ChunkExtract view
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "resource.h"
#include "Common\Util\IfcBroadcastObject.h"
#include "Common\Util\LogMessageObject.h"

//-----  CLASS  ---------------------------------------------------------------
class CFormChunkExtractView : public CFormView, public LogMessageObject, public IfcBroadcastObject
{
	private:
		CToolTipCtrl				_toolTipCtrl;
		CString						_fileNameIn;
		CString						_fileNameNif;
		CString						_fileNameObj;

		enum{ IDD = IDD_FORM_CHUNKEXTRACT };

	protected:
									CFormChunkExtractView();
		virtual	void				DoDataExchange(CDataExchange* pDX);
		virtual	void				OnInitialUpdate();
		virtual	BOOL				PreTranslateMessage(MSG* pMsg);

		afx_msg	HBRUSH				OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		afx_msg void				OnBnClickedBtViewIn();
		afx_msg void				OnBnClickedBtViewNif();
		afx_msg void				OnBnClickedBtViewObj();
		afx_msg void				OnBnClickedBtFileIn();
		afx_msg void				OnBnClickedBtFileNif();
		afx_msg void				OnBnClickedBtFileObj();
		afx_msg void				OnBnClickedBtResetForm();
		afx_msg void				OnBnClickedBtNscopeIn();
		afx_msg void				OnBnClickedBtNscopeNif();
		afx_msg void				OnBnClickedBtConvert();

		DECLARE_DYNCREATE(CFormChunkExtractView)
		DECLARE_MESSAGE_MAP()

	public:
		virtual						~CFormChunkExtractView();
		virtual	BOOL				PreCreateWindow(CREATESTRUCT& cs);

				CNifUtilsSuiteDoc*	GetDocument() const;

		//  IfcBroadcastObject
		virtual	BOOL				BroadcastEvent(WORD event, void* pParameter=NULL);

		//  IfcLogMessageObject
		virtual	void				LogMessage(const CString text, const CHARFORMAT* pFormat);
};

#ifndef _DEBUG  // debug version in NifUtilsSuiteView.cpp
inline CNifUtilsSuiteDoc* CFormChunkExtractView::GetDocument() const
   { return reinterpret_cast<CNifUtilsSuiteDoc*>(m_pDocument); }
#endif
