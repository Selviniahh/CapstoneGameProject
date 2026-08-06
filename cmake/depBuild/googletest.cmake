# GoogleTest — sadece Test/ altındaki unit test hedefi (ETGUnitTests) tarafından kullanılır.
# Diğer bağımlılıklar gibi git submodule olarak deps/googletest altında tutulur; ayrı bir
# depbuilt/ cache'i yok çünkü gtest küçük bir kütüphane, doğrudan add_subdirectory ile
# projenin içine derleniyor.
#
# Bu dosya include edildikten sonra ETG_GTEST_AVAILABLE değişkeni:
#   TRUE  -> GTest::gtest / GTest::gtest_main target'ları kullanılabilir
#   FALSE -> submodule çekilmemiş; çağıran taraf unit testleri sessizce devre dışı bırakmalı
#            (git submodule update --init deps/googletest)

if(TARGET GTest::gtest_main)
    # Zaten include edilmiş
    set(ETG_GTEST_AVAILABLE TRUE)
    return()
endif()

set(ETG_GTEST_DIR ${DEPS_SOURCE_DIR}/googletest)

if(NOT EXISTS ${ETG_GTEST_DIR}/CMakeLists.txt)
    set(ETG_GTEST_AVAILABLE FALSE)
    return()
endif()

# gtest'i her zaman STATIC derle. Projenin geri kalanı BUILD_SHARED_LIBS=ON ile shared
# derleniyor ama shared bir gtest, Windows'ta tüketicinin GTEST_LINKED_AS_SHARED_LIBRARY
# tanımlamasını gerektirir ve hiçbir kazancı yoktur: gtest yalnızca test binary'sine linklenir.
set(ETG_SAVED_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)

# MSVC'de gtest varsayılan olarak statik CRT kullanır, proje ise dinamik. İkisini eşitle.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# gmock'a ihtiyacımız yok ve gtest'i install etmiyoruz.
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

# EXCLUDE_FROM_ALL: unit testler kapalıyken "make all" gtest'i derlemeye kalkmasın.
add_subdirectory(${ETG_GTEST_DIR} ${CMAKE_BINARY_DIR}/deps/googletest EXCLUDE_FROM_ALL)

set(BUILD_SHARED_LIBS ${ETG_SAVED_BUILD_SHARED_LIBS})

set(ETG_GTEST_AVAILABLE TRUE)
