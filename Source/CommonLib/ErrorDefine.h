#pragma once

#pragma warning(disable:4100)

// 아래는 c++ 17 에서는 지원 안되는 기능을 사용한 코드들을 모두 에러로 처리되자 않도록 하는 선언문
// 부스트나 colony 같은 라이브러리에서 발생하고 있기 때문에
// 나중에 수정된 버전이 나오면 버전업을 하고 해당 선언문을 지워주도록 하자
#define _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING