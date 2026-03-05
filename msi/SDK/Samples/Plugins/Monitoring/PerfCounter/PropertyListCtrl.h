// PropertyListCtrl.h : header file
//
// created by Unwinder
/////////////////////////////////////////////////////////////////////////////
#ifndef _PROPERTYLISTCTRL_INCLUDED_
#define _PROPERTYLISTCTRL_INCLUDED_
/////////////////////////////////////////////////////////////////////////////
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/////////////////////////////////////////////////////////////////////////////
#include <afxtempl.h>

#include "IconImageList.h"
/////////////////////////////////////////////////////////////////////////////
#define UM_SEL_CHANGED							WM_APP + 103
/////////////////////////////////////////////////////////////////////////////
#define PLC_ITEM_FLAG_ROOT						0x00000001
#define PLC_ITEM_FLAG_ALLOW_ROOT_EXPAND			0x00000002
#define PLC_ITEM_FLAG_EXPANDED					0x00000008
#define PLC_ITEM_FLAG_LAST_ROOT					0x00000010
#define PLC_ITEM_FLAG_LAST_CHILD				0x00000020
#define PLC_ITEM_FLAG_HOT						0x00000040
#define PLC_ITEM_FLAG_TEXT_CALLBACK				0x00000080
#define PLC_ITEM_FLAG_PROPERTY_TEXT_CALLBACK	0x00000100
#define PLC_ITEM_FLAG_PROPERTY_COLUMN			0x00000200
#define PLC_ITEM_FLAG_SELECTED					0x00000400
/////////////////////////////////////////////////////////////////////////////
typedef struct PLC_ITEM_DESC 
{
	char			szText        [MAX_PATH];
	char			szPropertyText[MAX_PATH];

	DWORD			dwFlags;

	DWORD			dwUserData0;
	DWORD			dwUserData1;
	DWORD			dwUserData2;
	DWORD			dwUserData3;

	PLC_ITEM_DESC*	pRoot;
	
} PLC_ITEM_DESC;
/////////////////////////////////////////////////////////////////////////////
class CPropertyListCtrl : public CListCtrl
{
public:

	void			SetResourceHandle(HINSTANCE hModule);
	virtual void	Init();
	void			Uninit();

	void			AllowEmptyRootExpand(BOOL bAllowEmptyRootExpand);
	void			AllowItemSelection(BOOL bAllowItemSelection);
	void			DrawRootButtons(BOOL bDrawRootButtons);
	void			DrawPropertyCells(BOOL bDrawPropertyCells);
	void			EnableMultiSelection(BOOL bEnableMultiSelection);
	void			EnableItemEdit(BOOL bEnableItemEdit);

	BOOL			IsHeaderDragging();
	int				GetFocusedItem();
	int				GetSelCount();

	PLC_ITEM_DESC*	InsertItem(DWORD dwFlags, LPCSTR lpszText = NULL, LPCSTR lpszPropertyText = NULL, PLC_ITEM_DESC *pRoot = NULL, DWORD dwUserData0 = 0, DWORD dwUserData1 = 0, DWORD dwUserData2 = 0, DWORD dwUserData3 = 0);
	PLC_ITEM_DESC*	GetItemDesc(int iItem);

	CList<PLC_ITEM_DESC*,PLC_ITEM_DESC*>* GetItemsList();

	virtual CString GetPropertyTextCallback(int iItem, int iColumn);
	virtual void	OnSelectionChanged(int nSel);
	virtual void	OnDblClkItem(int iItem, int iSubItem);
	virtual void	OnCollapseItem(int iItem);
	virtual void	OnExpandItem(int iItem);
	virtual void	OnCheckItem(int iItem);
	virtual void	DoEdit(int iItem, int iSubItem);
	virtual BOOL	IsEditing();
	virtual void	UndoEdit();
	virtual void	OnEndLabelEdit(NMHDR* pNMHDR);

	void			RedrawVisibleItems();

	inline BOOL IsValidItem(int iItem) 
		{ return ((iItem >= 0) && (iItem < GetItemCount())); }
	inline BOOL IsValidItemDesc(PLC_ITEM_DESC* pItemDesc) 
		{ return (pItemDesc != NULL); }

	CPropertyListCtrl();
	virtual ~CPropertyListCtrl();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertyListCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

protected:
	void	PrepareMemDC(CDC* pDC);
	void	DestroyMemDC();

	void	DrawCheck(CDC *pDC, CRect rect, BOOL bChecked);
	void	DrawRootButton(CDC* pDC, CRect rect, BOOL bExpanded);
	void	DrawItem(int iItem, DWORD itemState, CDC* pDC, CRect itemRect, BOOL bDrawAlways = FALSE);

	void	TrackSelectionChange();
	void	SelectNext();
	void	ToggleCurSel();
	void	ExpandItem(int iItem);
	void	CollapseItem(int iItem);

	int		HitTestOnItem(CPoint point);
	int		HitTestOnSubitem(CPoint point);
	int		HitTestOnPlus(CPoint point);
	int		HitTestOnCheck(CPoint point);

	CRect	GetItemsRect();

	void	InvalidateBgndRgn();
	void	GetBgndRgn(CRgn &bgndRgn);

	DWORD	Blend(DWORD dwColor0, DWORD dwColor1, float fltRatio);
	LPCTSTR MakeShortString(CDC *pDC, LPCTSTR lpszLong, int nColumnLen, int nOffset);

	int		m_iSelectedItem;

	BOOL	m_bAllowEmptyRootExpand;
	BOOL	m_bAllowItemSelection;
	BOOL	m_bDrawRootButtons;
	BOOL	m_bDrawPropertyCells;
	BOOL	m_bEnableMultiSelection;
	BOOL	m_bEnableItemEdit;

	BOOL	m_bHeaderDragging;

	CDC		m_memDC;
	CBitmap m_bitmap;

	CList<PLC_ITEM_DESC*,PLC_ITEM_DESC*>	m_itemsList;
	CIconImageList							m_imageList;

	//{{AFX_MSG(CPropertyListCtrl)
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
	//}}AFX_MSG
	afx_msg void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
/////////////////////////////////////////////////////////////////////////////
#endif 
/////////////////////////////////////////////////////////////////////////////
