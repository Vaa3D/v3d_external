#ifdef _ENABLE_MACX_DRAG_DROP_FIX_

#include <QUrl>
#include "v3d_core.h"
#import <Foundation/NSURL.h>

// @ADDED by Alessandro on 2015-05-09. Method to get the path-based URL from the file-based URL
QString getPathFromYosemiteFileReferenceURL(QUrl url)
{
    // get QString
    QString path = url.toString();

    // for memory allocation
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    // convert QString to NSString
    NSString *fileIdURL = [[NSString alloc] initWithCString:path.toUtf8() encoding:NSASCIIStringEncoding];

    // fix the URL
    NSURL *goodURL = [[NSURL URLWithString:fileIdURL] filePathURL];

    // convert fixed URL to NSString
    NSString *myString = [goodURL absoluteString];

    // convert NSString to QString
    QString result([myString cStringUsingEncoding:NSUTF8StringEncoding]);

    // get rid of file://
    result = result.mid(7);

    // release memory
    [fileIdURL release];
    [pool release];

    return result;
}

#endif
