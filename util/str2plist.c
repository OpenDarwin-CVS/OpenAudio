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
	
int main (int argc, const char **argv) {
  if (argc == 3) {
    CFPropertyListRef propertyList =
      CreateMyPropertyListFromFile(argv[1]);
    
    if (!propertyList) {
      printf("Failed to create property list from %s!\n", argv[1]);
    }

    WriteMyPropertyListToFile(propertyList, argv[2]);
    CFRelease(propertyList);
  } else {
    printf(":(\n");
  }

  return 0;
}
