#include "WindowLoopManager.h"
#include "Resource.h"


WindowLoopManager::WindowLoopManager()
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
WindowLoopManager::~WindowLoopManager()
{
}



void WindowLoopManager::Initialize(const HINSTANCE& hInstance)
{
	m_hInstance = hInstance;
}

HRESULT WindowLoopManager::AllocWindow(const std::string_view& windowTitle, DWORD windowStyle, LRESULT(CALLBACK* wndProc)(HWND, UINT, WPARAM, LPARAM))
{
	m_wndStyle = windowStyle;

	using WindowClassEx_t = WNDCLASSEXA;
	auto RegisterClassEx_f = RegisterClassExA;
	auto CreateWindowEx_f = CreateWindowExA;

	//Register Window
	WindowClassEx_t wcex;
	wcex.cbSize = sizeof(WindowClassEx_t);
	wcex.hInstance = m_hInstance;
	wcex.lpszClassName = windowTitle.data();
	wcex.lpfnWndProc = wndProc;

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;//CS_SAVEBITS | 
	wcex.lpszMenuName = NULL;
	wcex.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(107));
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	
	wcex.cbClsExtra = NULL;
	wcex.cbWndExtra = NULL;
	RegisterClassEx_f(&wcex);

	//Create Window
	if (!(m_hWnd = CreateWindowEx_f(0L, windowTitle.data(), windowTitle.data(), m_wndStyle,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, m_hInstance, nullptr)))
		return E_FAIL;

	return S_OK;
}

void WindowLoopManager::WndMove(const int & xPos, const int & yPos)
{
	SetWindowPos(m_hWnd, NULL, xPos, yPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void WindowLoopManager::WndResize(const unsigned int & xSize, const unsigned int & ySize)
{
	RECT temp;
	SetRect(&temp, 0, 0, xSize, ySize);
	AdjustWindowRect(&temp, m_wndStyle, NULL);
	SetWindowPos(m_hWnd, NULL, 0, 0, temp.right - temp.left, temp.bottom - temp.top, SWP_NOMOVE | SWP_NOZORDER);
}



WPARAM WindowLoopManager::Loop()
{
	using namespace std::chrono_literals;


	//Excution Initialize
	if (m_initialize)
		m_initialize();

	ClockTimePoint_t prev_update = Clock_t::now();
	ClockTimePoint_t prev_render = Clock_t::now();

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
