#include <CoreFoundation/CoreFoundation.h>

CFPropertyListRef CreateMyPropertyListFromFile(const char *url);
void WriteMyPropertyListToFile(CFPropertyListRef propertyList,
			       const char *url);
	
int main (int argc, const char **argv) {
#if 0
  CFPropertyListFormat format = kCFPropertyListXMLFormat_v1_0;

  if (argc > 2 && argv[1][0] == '-') {
    switch (argv[0][1]) {
    case 'b':
      format = kCFPropertyListBinaryFormat_v1_0;
      break;
    case 'o':
      format = kCFPropertyListOpenStepFormat;
      break;
    case 'x':
      format = kCFPropertyListXMLFormat_v1_0;
      break;
    default:
      argc++;
      argv--;
    }

    argc--;
    argv++;
  }
#endif  

  if (argc == 3) {
    CFPropertyListRef propertyList =
      CreateMyPropertyListFromFile(argv[1]);
    WriteMyPropertyListToFile(propertyList, argv[2]);
    CFRelease(propertyList);
  } else {
    printf(":(\n");
  }

  return 0;
}
void WriteMyPropertyListToFile(CFPropertyListRef propertyList,
			       const char *url) {
  CFDataRef xmlData;
  Boolean status;
  SInt32 errorCode;	

  CFStringRef str = 
    CFStringCreateWithCString(kCFAllocatorDefault, 
			      url, kCFStringEncodingUTF8);
  CFURLRef fileURL =
    CFURLCreateWithFileSystemPath(kCFAllocatorDefault, str,
				  kCFURLPOSIXPathStyle, false);

  // Convert the property list into XML data.
  xmlData = 
    CFPropertyListCreateXMLData(kCFAllocatorDefault, propertyList);
	

  // Write the XML data to the file.
  status = CFURLWriteDataAndPropertiesToResource(fileURL,
						  xmlData,
						  NULL,
						  &errorCode);

  CFRelease(fileURL);
  CFRelease(str);
  CFRelease(xmlData);
}
	

CFPropertyListRef CreateMyPropertyListFromFile(const char *url) {
  CFPropertyListRef propertyList;
  CFStringRef       errorString;
  CFDataRef         resourceData;
  Boolean           status;
  SInt32            errorCode;

  CFStringRef str = 
    CFStringCreateWithCString(kCFAllocatorDefault, 
			      url, kCFStringEncodingUTF8);
  CFURLRef fileURL =
    CFURLCreateWithFileSystemPath(kCFAllocatorDefault, str,
				  kCFURLPOSIXPathStyle, false);

  // Read the XML file.
  status = 
    CFURLCreateDataAndPropertiesFromResource(kCFAllocatorDefault,
					     fileURL,
					     &resourceData,
					     NULL,      
					     NULL,
					     &errorCode);
	

  // Reconstitute the dictionary using the XML data.
  propertyList = 
    CFPropertyListCreateFromXMLData(kCFAllocatorDefault,
				    resourceData,
				    kCFPropertyListImmutable,
				    &errorString);
	

  CFRelease(resourceData);
  CFRelease(fileURL);
  CFRelease(str);

  return propertyList;
}
