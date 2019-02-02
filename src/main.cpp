#include "mainwindow.h"
#include "network/imageworker.h"
#include "cxxopts/cxxopts.hpp"

#include <log4cxx/logger.h>

#include <log4cxx/basicconfigurator.h>
#include <log4cxx/mdc.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/helpers/properties.h>
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

    log4cxx::MDC::put("pid", std::to_string(getpid()));
//    log4cxx::helpers::Properties properties;
//    properties.put("log4j.rootCategory", "INFO, stdout, stderr, file");
//    properties.put("log4j.appender.stdout", "org.apache.log4j.ConsoleAppender");
//    properties.put("log4j.appender.stdout.layout", "org.apache.log4j.PatternLayout");
//    properties.put("log4j.appender.stdout.layout.ConversionPattern", "%m%n");
//    properties.put("log4j.appender.stdout.filter.filter1", "org.apache.log4j.filter.LevelMatchFilter");
//    properties.put("log4j.appender.stdout.filter.filter1.levelToMatch", "INFO");
//    properties.put("log4j.appender.stdout.filter.filter1.acceptOnMatch", "true");
//    properties.put("log4j.appender.stdout.filter.filter2", "org.apache.log4j.filter.DenyAllFilter");
//    properties.put("log4j.appender.stderr", "org.apache.log4j.ConsoleAppender");
//    properties.put("log4j.appender.stderr.Target", "System.err");
//    properties.put("log4j.appender.stderr.Threshold", "WARN");
//    properties.put("log4j.appender.stderr.layout", "org.apache.log4j.PatternLayout");
//    properties.put("log4j.appender.stderr.layout.ConversionPattern", "%m%n");
//    properties.put("log4j.appender.file", "org.apache.log4j.RollingFileAppender");
//    properties.put("log4j.appender.file.layout", "org.apache.log4j.PatternLayout");
//    properties.put("log4j.appender.file.File", "/var/lib/ImageDownloader/Imagedownloader.log");
//    properties.put("log4j.appender.file.MaxFileSize", "100KB");
//    //properties.put("log4j.appender.file.layout.ConversionPattern", "%clr(%d{yyyy-MM-dd HH:mm:ss,SSS}){faint} %clr(%5p) %clr( ){magenta} %clr(---){faint} %clr([%15.15t]){faint} %clr(%-40.40logger{39}){cyan} %clr(:){faint} %m%n");
//    properties.put("log4j.appender.file.layout.ConversionPattern", "%d{yyyy-MM-dd HH:mm:ss.SSS} %5p  %X{pid} --- [%14t] %-12.12logger{39} : %m%n");
//    //properties.put("log4j.appender.file.layout.ConversionPattern", "\u001b[0;2m%d{yyyy-MM-dd HH:mm:ss.SSS}\u001b[m %5p  %X{pid} \u001b[0;2m---\u001b[m \u001b[0;2m[%14t]\u001b[m \u001b[0;36m%-32.32F:%L{39}\u001b[m \u001b[0;2m:\u001b[m %m%n");
//    log4cxx::PropertyConfigurator::configure(properties);
    log4cxx::xml::DOMConfigurator::configure("/var/lib/ImageDownloader/log4cxx.xml");

    MainWindow w(tags);
    w.show();

    return a.exec();
}
