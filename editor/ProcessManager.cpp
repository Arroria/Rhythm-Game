#include "WindowLoopManager.h"
#include "Resource.h"


ProcessManager::ProcessManager()
	: m_hInstance(nullptr)
	, m_hWnd(nullptr)
	, m_wndStyle(NULL)

	, m_initialize(nullptr)
	, m_update(nullptr)
	, m_render(nullptr)
	, m_release(nullptr)

	, m_updateFPSLimite(NULL)
	, m_renderFPSLimite(NULL)
	, m_deltatime_update(NULL)
	, m_deltatime_render(NULL)
{
}
ProcessManager::~ProcessManager()
{
}



void ProcessManager::Initialize(const HINSTANCE& hInstance)
{
	m_hInstance = hInstance;
}

HRESULT ProcessManager::AllocWindow(const std::string_view& windowTitle, DWORD windowStyle, LRESULT(CALLBACK* wndProc)(HWND, UINT, WPARAM, LPARAM))
{
	m_wndStyle = windowStyle;


	//Register Window
	WNDCLASSEXA wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = wndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = m_hInstance;
	wcex.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(107));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL; // MAKEINTRESOURCEW(IDC_SUBMARINE);
	wcex.lpszClassName = windowTitle.data();
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	RegisterClassExA(&wcex);

	//Create Window
	if (!(m_hWnd = CreateWindowExA(0L, windowTitle.data(), windowTitle.data(), m_wndStyle,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, m_hInstance, nullptr)))
		return E_FAIL;

	//ShowWindow(m_wndInfo->hWnd, SW_RESTORE);
	////UpdateWindow(m_wndInfo->hWnd);
	//
	//m_wndInfo->name = wndName;
	//m_wndInfo->wndStyle = wndStyle;
	return S_OK;
}

void ProcessManager::WndMove(const int & xPos, const int & yPos)
{
	SetWindowPos(m_hWnd, NULL, xPos, yPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void ProcessManager::WndResize(const unsigned int & xSize, const unsigned int & ySize)
{
	RECT temp;
	SetRect(&temp, 0, 0, xSize, ySize);
	AdjustWindowRect(&temp, m_wndStyle, NULL);
	SetWindowPos(m_hWnd, NULL, temp.left, temp.top, temp.right, temp.bottom, SWP_NOMOVE | SWP_NOZORDER);
}



WPARAM ProcessManager::Loop(bool autoClear)
{
	using namespace std::chrono_literals;
	ClockTimePoint_t prev_update;
	ClockTimePoint_t prev_render;


	//Excution Initialize
	if (m_initialize)
		m_initialize();

	MSG msg;
	PeekMessageW(&msg, NULL, NULL, NULL, PM_NOREMOVE);
	while (msg.message != WM_QUIT)
	{
		//Excution WndProc
		if (PeekMessageW(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			//Maybe do not need this//	
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else
		{
			ClockTimePoint_t current = Clock_t::now();

			//Update
			if (m_update)
			{
				m_deltatime_update = current - prev_update;
				if (!m_updateFPSLimite || (m_deltatime_update * m_updateFPSLimite >= ClockTime_t(1s)))
				{
					prev_update = current;
					m_update();
				}
			}

			//Render
			if (m_render)
			{
				m_deltatime_render = current - prev_render;
				if (!m_renderFPSLimite || m_deltatime_render * m_renderFPSLimite >= ClockTime_t(1s))
				{
					prev_render = current;
					m_render();
				}
			}
		}
	}

	//Excution Release
	if (m_release)
		m_release();

	return msg.wParam;
}
