#pragma once

#pragma warning(disable:4100)
#pragma warning(disable:4201)

// 아래는 c++ 17 에서는 더이상 지원 안하는 기능을 사용한 코드들을 에러로 처리되지 않도록 하는 선언문
// 부스트나 colony 같은 라이브러리에서 발생하고 있기 때문에
// 나중에 수정된 버전이 나오면 버전업을 하고 해당 선언문을 지워주도록 하자
//#define _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING
//#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

#if defined(_CRTDBG_MAP_ALLOC) && (defined(DEBUG) || defined(_DEBUG))
#define _CRTDBG_MAP_ALLOC	// 메모리 누수 감지
#endif

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#define NOMINMAX

#include <stdlib.h>
#include <crtdbg.h>