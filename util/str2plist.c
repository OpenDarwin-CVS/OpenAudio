/*
 * Copyright (c) 2004 Dan Villiom Podlaski Christiansen <danchr@daimi.au.dk>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
