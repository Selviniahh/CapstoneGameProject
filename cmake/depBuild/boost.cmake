# Boost (header-only: type_index, describe): sadece kullanılan kütüphaneler ve
# bunların transitive header bağımlılıkları resmi Boost Git submodule'ları olarak
# deps/boost altında tutulur. Tüm Boost superproject'ini indirmiyoruz.
#Boost header-only olduğu için en başından PUBLIC yapabileceğimiz
#bir target türü yok.

#> Dosya üretmeyen, sahte bir Boost target’ı oluştur.
add_library(boost_header_only INTERFACE) # Target uretme 

#> Bu target’ı kullananlara gerekli Boost modüllerinin include klasörlerini
#> ver. Boost header’larından gelen uyarıları da SYSTEM sayesinde sustur.
#SYSTEM: Compiler'a bu external hatalari gosterme diyebiliyoruz
target_include_directories(boost_header_only SYSTEM INTERFACE
        ${DEPS_SOURCE_DIR}/boost/config/include
        ${DEPS_SOURCE_DIR}/boost/assert/include
        ${DEPS_SOURCE_DIR}/boost/mp11/include
        ${DEPS_SOURCE_DIR}/boost/describe/include
        ${DEPS_SOURCE_DIR}/boost/exception/include
        ${DEPS_SOURCE_DIR}/boost/throw_exception/include
        ${DEPS_SOURCE_DIR}/boost/container_hash/include
        ${DEPS_SOURCE_DIR}/boost/type_index/include
)

add_library(Boost::type_index ALIAS boost_header_only)
add_library(Boost::describe ALIAS boost_header_only)
