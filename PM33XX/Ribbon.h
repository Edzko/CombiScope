#include <uiribbon.h>

class CListHandler
    : public IUICommandHandler // Command handlers must implement IUICommandHandler.
{
public:
    STDMETHOD(Execute)(UINT nCmdID,
                       UI_EXECUTIONVERB verb, 
                       const PROPERTYKEY* key,
                       const PROPVARIANT* ppropvarValue,
                       IUISimplePropertySet* pCommandExecutionProperties);

    STDMETHOD(UpdateProperty)(UINT nCmdID,
                              REFPROPERTYKEY key,
                              const PROPVARIANT* ppropvarCurrentValue,
                              PROPVARIANT* ppropvarNewValue);

    static HRESULT CreateInstance(CListHandler **ppHandler, int id);

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID iid, void** ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

	void Update();
	//bool myTaskUpdate;
	int ID;
    HRESULT CreateUIImageFromBitmapResource(LPCTSTR pszResource, IUIImage **ppimg);

private:
    CListHandler(int id)
        : m_cRef(1), m_pifbFactory(NULL)
    {
		ID = id;
		//myTaskUpdate = false;
    }

    IUIImageFromBitmap* m_pifbFactory;
    LONG m_cRef;
};

extern const char BaudRates[5][20];
extern	int SelectedBaudrate, SelectedComport, ComData[50];

extern int topMargin;
extern IUIFramework *g_pFramework;
extern CListHandler *pTaskHandler;
extern CListHandler *pAVCTaskHandler;
extern CListHandler *pOperationHandler;
