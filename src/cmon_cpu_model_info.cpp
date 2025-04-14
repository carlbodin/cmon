#include <Wbemidl.h> // For IWbemLocator and IWbemServices
#include <iostream>

// Helper function to convert BSTR to std::string
std::string bSTRToString(BSTR bstr) {
  int wslen = SysStringLen(bstr);
  int len = WideCharToMultiByte(CP_ACP, 0, bstr, wslen, NULL, 0, NULL, NULL);
  std::string str(len, '\0');
  WideCharToMultiByte(CP_ACP, 0, bstr, wslen, &str[0], len, NULL, NULL);
  return str;
}

void getCpuInfoDetails(std::string &processorName) {
  // Initialize COM
  HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
  if (FAILED(hres)) {
    std::cerr << "Failed to initialize COM library. Error code: " << hres << std::endl;
    return;
  }

  // Set general COM security levels
  hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
                              RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
  if (FAILED(hres)) {
    std::cerr << "Failed to initialize security. Error code: " << hres << std::endl;
    CoUninitialize();
    return;
  }

  // Obtain the initial locator to WMI
  IWbemLocator *pLoc = NULL;
  hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator,
                          (LPVOID *)&pLoc);
  if (FAILED(hres)) {
    std::cerr << "Failed to create IWbemLocator object. Error code: " << hres
              << std::endl;
    CoUninitialize();
    return;
  }

  // Connect to the WMI namespace
  IWbemServices *pSvc = NULL;
  hres = pLoc->ConnectServer(SysAllocString(L"ROOT\\CIMV2"), // WMI namespace
                             NULL,                           // User name
                             NULL,                           // User password
                             0,                              // Locale
                             0,                              // Security flags
                             0,                              // Authority
                             0,                              // Context object
                             &pSvc                           // IWbemServices proxy
  );
  if (FAILED(hres)) {
    std::cerr << "Could not connect to WMI namespace. Error code: " << hres
              << std::endl;
    pLoc->Release();
    CoUninitialize();
    return;
  }

  // Set security levels on the proxy
  hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                           RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                           EOAC_NONE);
  if (FAILED(hres)) {
    std::cerr << "Could not set proxy blanket. Error code: " << hres << std::endl;
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
    return;
  }

  // Query for processor name
  IEnumWbemClassObject *pEnumerator = NULL;
  hres = pSvc->ExecQuery(
      SysAllocString(L"WQL"), SysAllocString(L"SELECT Name FROM Win32_Processor"),
      WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
  if (FAILED(hres)) {
    std::cerr << "Query for processor name failed. Error code: " << hres << std::endl;
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
    return;
  }

  IWbemClassObject *pclsObj = NULL;
  ULONG uReturn = 0;

  // Get the processor name
  while (pEnumerator) {
    HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
    if (0 == uReturn) {
      break;
    }

    VARIANT vtProp;
    hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
    if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) {
      processorName = bSTRToString(vtProp.bstrVal);
    }
    VariantClear(&vtProp);
    pclsObj->Release();
  }
  pEnumerator->Release();

  // Cleanup
  pSvc->Release();
  pLoc->Release();
  CoUninitialize();
}
