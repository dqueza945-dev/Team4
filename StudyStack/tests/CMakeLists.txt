find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Test Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test Sql)

add_executable(StudyStackTests
    test_database.cpp
    ../database.cpp
)

target_link_libraries(StudyStackTests PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Sql
)

add_executable(TestLogin
    test_login.cpp
    ../database.cpp
)

target_link_libraries(TestLogin PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Sql
)

add_executable(TestSignUp
    test_signup.cpp
    ../database.cpp
)

target_link_libraries(TestSignUp PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Sql
)

add_executable(TestTasks
    test_tasks.cpp
    ../database.cpp
)

target_link_libraries(TestTasks PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Sql
)

add_executable(TestEdit
    test_edit.cpp
    ../database.cpp
)

target_link_libraries(TestEdit PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Sql
)

add_executable(TestWarning
    test_warning.cpp
)

target_link_libraries(TestWarning PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
)

add_executable(TestValidation
    test_validation.cpp
)

target_link_libraries(TestValidation PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
)