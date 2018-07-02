
#include "exception_errno.h"
#include "soxhelper.h"

namespace sox
{

const std::string & exception_errno::what_str() const throw()
{
	if (m_bstrerror)
		return m_what;

	m_bstrerror = true;

	if (0 == what_errno())
		return m_what;

	m_what += " - ";
	m_what += ultoa10(what_errno()) + ": ";

#if defined(WIN32)

	HLOCAL hlocal = NULL;	// Buffer that gets the error message string
	// Get the error code's textual description
	BOOL fOk = FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, what_errno(), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		(PTSTR) &hlocal, 0, NULL);

	if (!fOk)
	{
		// Is it a network-related error?
		HMODULE hDll = LoadLibraryEx(TEXT("netmsg.dll"), NULL, DONT_RESOLVE_DLL_REFERENCES);
		if (hDll != NULL)
		{
			FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM,
				hDll, what_errno(), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
				(PTSTR) &hlocal, 0, NULL);
			FreeLibrary(hDll);
		}
	}

	if (hlocal != NULL)
	{
		m_what += (char *) LocalLock(hlocal);
		LocalFree(hlocal);
	}
	else
	{
		m_what += "No Error Text.";
	}
#else
	// XXX thread safe
	m_what += strerror(what_errno());
#endif

	return m_what;
}

} // namespace sox
