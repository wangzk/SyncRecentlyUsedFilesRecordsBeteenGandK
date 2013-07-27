//GTK Headers should be placed before Qt's
#include <gtk/gtk.h>
#include <QCoreApplication>
#include <KUrl>
#include <KDebug>
#include <KDesktopFile>
#include <QDir>
#include <QFile>
#include <QString>
#include <QDateTime>
#include <QList>
#include <iostream>
#include <fstream>
#include <set>
#include <unistd.h>
#include <sys/wait.h>
#include <cassert>
#include <kconfiggroup.h>
#include "krecentdocument.h"
using namespace std;


struct BookmarkInfo
{
    KUrl url;
    QDateTime added,visited,modified;

    bool operator==(const BookmarkInfo& b) const
    {
        return (this->url == b.url);
    }
    bool operator<(const BookmarkInfo& b) const
    {
        return (url < b.url);
    }
};


const QString BookMarkMark("<bookmark href=\"");
const QString TIME_FORMAT("yyyy-MM-ddTHH:mm:ssZ");
GtkRecentManager* gtkRecentManager;

/**
 * @brief readAttribute Read the attribution from the src line
 * @param dest write the attribute into it
 * @param AtrributeName
 * @param src src_line
 * @return success?
 */
bool readAttribute(QString& dest, const QString AttributeName, QString src, int StartPos)
{
    int startPos = src.indexOf(AttributeName,StartPos);

    if(startPos < 0) return false;
    startPos = startPos + AttributeName.length() + 2;//Skip ``AttributeName=\" ''

    int stopPos = src.indexOf(QString('"'),startPos);
    if(stopPos < 0) return false;//The src is not finished
    dest = src.mid(startPos,stopPos - startPos);

    return true;
}

/**
 * Read the GTK RU Records from GtkRecentManager and update the `records`
 * @return the new records appearing in GTK+ bookmark file
 */
QList<BookmarkInfo> ReadGTKRURecords(set<BookmarkInfo>& records)
{
    QList<BookmarkInfo> newRecords;
    set<BookmarkInfo> currentRecords;
    GList* gtkRURecords;
    gtkRURecords = gtk_recent_manager_get_items(gtkRecentManager);
    GList* p = gtkRURecords;
// qDebug() << "GTK Records:";
    while(p != NULL)
    {
        BookmarkInfo binfo;
        GtkRecentInfo* info = static_cast<GtkRecentInfo*>(p->data);
        const char* uri = gtk_recent_info_get_uri(info);
        binfo.url = KUrl(uri);
        QFileInfo finfo(binfo.url.toLocalFile());
        if (!finfo.exists())
        {
            //Delete from Recently used files list
            GError* error = NULL;
            gtk_recent_manager_remove_item(gtkRecentManager,uri,&error);
            if(error)
            {
                g_warning("fail to delete %s",error->message);
                g_error_free(error);
            }
            qDebug() << "File no exist:" <<  binfo.url;
            gtk_recent_info_unref(info);
            p = p->next;
            continue;
        }
        binfo.added = QDateTime::fromTime_t(gtk_recent_info_get_added(info));
        binfo.modified = QDateTime::fromTime_t(gtk_recent_info_get_modified(info));
        binfo.visited = QDateTime::fromTime_t(gtk_recent_info_get_visited(info));
//   qDebug() << binfo.url;

        if(records.find(binfo) == records.end())
        {
            newRecords.push_back(binfo);
        }
        currentRecords.insert(binfo);
        gtk_recent_info_unref(info);
        p = p->next;
    }
    g_list_free(gtkRURecords);
    records = currentRecords;//Update the outside records
    return newRecords;
}


/**
 * Read the KDE RU Records and upate the argument `records`
 * @return the new records appearing in KDE Recent documents
 */
QList<BookmarkInfo> ReadKDERURecords(set<BookmarkInfo>& records)
{
    QStringList desktopFiles;
    desktopFiles = KRecentDocument::recentDocuments();//get the desktop files of KDE RU Records

    QList<BookmarkInfo> newRecord;//for return
    set<BookmarkInfo> currentRecords;

    foreach(QString filePath,desktopFiles)
    {
        KDesktopFile tmp(filePath);
        QString urlString;
        urlString = tmp.desktopGroup().readPathEntry("URL",QString());
        KUrl url(urlString);
        QFileInfo finfo(filePath);

        BookmarkInfo binfo;
        binfo.url = url;
        binfo.added = finfo.created();
        binfo.visited = finfo.lastRead();
        binfo.visited = binfo.modified = finfo.lastModified();
        //KDE RU uses file info to records those times

        if(records.find(binfo) == records.end())
        {
            newRecord.push_back(binfo);
        }
        currentRecords.insert(binfo);
    }
    records = currentRecords;//update the records outside
    return newRecord;
}


/*
 * Insert the RU records in newRecords into the GTK+ Xbel file
 */
void InsertIntoGTKRecords(QList<BookmarkInfo> newRecords)
{
    foreach(BookmarkInfo b,newRecords)
    {

        //Get the char* presentation of QString url
        QByteArray uri8Bit = b.url.url().toLocal8Bit();
        const char* buri = uri8Bit.data();
        //Insert into GTK by gtk_recent_manager
        gboolean success;
        success = gtk_recent_manager_add_item(gtkRecentManager,buri);
        // qDebug() << "New GTK Bookmark:" << b.url;
        //qDebug() << "Insert into GTK+ " << b.url;
        cerr << "insert into GTK:" << buri << ":" << success << endl;
    }
}


int sleepTime = 2;/* at default, sleep two seconds */

int main(int argc, char** argv)
{

    if(argc >= 2) sleepTime = QString(argv[1]).toInt();

    set<KUrl> bookmarkSet;
    set<BookmarkInfo> KDEbookmarks,GTKbookmarks;

    while(1) {

        //Start the GTK main loop
        //gtk_main_iteration_do(false);//None block
        gtkRecentManager = gtk_recent_manager_new();
        QList<BookmarkInfo> gtkNew,kdeNew;
        gtkNew = ReadGTKRURecords(GTKbookmarks);
        kdeNew = ReadKDERURecords(KDEbookmarks);
        g_object_unref(gtkRecentManager);
        //Synchronize the KDE RU Records with GTK+
        foreach(BookmarkInfo b,gtkNew)
        {
            //For each gtk new records,check whether it is already in KDE Rencent documents
            if(KDEbookmarks.find(b) == KDEbookmarks.end())
            {
                KRecentDocument::add(b.url,QString("xdg-open"));
                KDEbookmarks.insert(b);
                qDebug() << "insert into KDE" << b.url;
            }
        }

        gtk_init(&argc,&argv);
        //Synchronize the GTK+ RU records with KDE
        QList<BookmarkInfo> insertIntoGTK;
        foreach(BookmarkInfo b,kdeNew)
        {
            if(GTKbookmarks.find(b) == GTKbookmarks.end())
            {
                GTKbookmarks.insert(b);
                insertIntoGTK.push_back(b);
            }
        }
        gtk_main_iteration_do(false);//GTKRecentManager must work with GTK+ main loop
        gtkRecentManager = gtk_recent_manager_new();
        InsertIntoGTKRecords(insertIntoGTK);
        gtk_main_iteration_do(false);
        g_object_unref(gtkRecentManager);
        while(gtk_events_pending()) gtk_main_iteration();
        gtk_main_iteration_do(false);//Make the GTK+ to write the file

        sleep(sleepTime);
    }

}
