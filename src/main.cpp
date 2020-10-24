#include "cxxopts/cxxopts.hpp"
#include "mainwindow.h"
#include "network/imageworker.h"

#include <memory>
#include <unistd.h>

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationDomain("ckoomen.eu");
    QCoreApplication::setOrganizationName("Independent");
    QCoreApplication::setApplicationName("ImageDownloader");
    QCoreApplication::setApplicationVersion("1.0.0");

    bool needHelp = false;
    bool printVersion = false;
    std::vector<std::string> tags;

    cxxopts::Options options(argv[0]);
    options.positional_help("[tag]");
    options.add_options()
            ("h, help", "Get help message", cxxopts::value<bool>(needHelp))
            ("v, version", "Get version", cxxopts::value<bool>(printVersion))
            ("tag", "Tag to narrow the search", cxxopts::value<std::vector<std::string>>(tags));
    options.parse_positional("tag");
    options.parse(argc, argv);
    if (needHelp)
    {
        puts(options.help().c_str());
        return 0;
    }
    if (printVersion)
    {
        puts("ImageDownloader 1.0.0");
        return 0;
    }

    MainWindow w(tags);
    w.show();

    return QApplication::exec();
}
