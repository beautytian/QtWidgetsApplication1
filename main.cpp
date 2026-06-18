#include <QApplication>

#include "MainWindow.h"
#include <QLocale>
#include <QTranslator>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("HGT search tools");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("DEMQuery");

    // 加载翻译（可选）
    // QTranslator translator;
    // if (translator.load(QLocale(), "dem_query_qt", "_", ":/translations")) {
    //     app.installTranslator(&translator);
    // }

    MainWindow window;
    window.show();

    return app.exec();
}