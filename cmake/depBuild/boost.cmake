# Boost (header-only: type_index, describe): sadece kullanılan kütüphanelerin
# ve bunların transitive header bağımlılıklarının (config, assert, mp11,
# container_hash, throw_exception, exception) tam kapanışı deps/boost
# altında vendored olarak tutuluyor — tüm Boost ağacı değil.
#Boost header-only olduğu için en başından PUBLIC yapabileceğimiz
#bir target türü yok.

#> Dosya üretmeyen, sahte bir Boost target’ı oluştur.
add_library(boost_header_only INTERFACE) # Target uretme 

#> Bu target’ı kullananlara Boost klasörünü include yolu olarak
#> ver. Boost header’larından gelen uyarıları da SYSTEM sayesinde
#> sustur.
#SYSTEM: Compiler'a bu external hatalari gosterme diyebiliyoruz
target_include_directories(boost_header_only SYSTEM INTERFACE ${DEPS_SOURCE_DIR}/boost) #include yolunu kullananlara ver

add_library(Boost::type_index ALIAS boost_header_only)
add_library(Boost::describe ALIAS boost_header_only)
