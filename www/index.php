<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd"> <!--*- html -*-->
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
  <head>
    <!--
	OpenAudio - openaudio
	
	Copyright (c) 2003-2004 Dan Christiansen
	
	Permission is hereby granted to use any portion of this page, for any
	purpose whatsoever.
	
	$Id$

	All email addresses on this page have been scrambled to prevent email
	harvesters from recognising them. See the PHP source for details. It's in
	www/index.php in the CVS repository.
	
	NOTE: I don't know HTML, so this code may be ugly. 
      -->
    <meta http-equiv="content-type" content="text/html; charset=iso-8859-1" />
    <meta name="author" content="Dan Christiansen" />
    
    <title>OpenAudio</title><link rel="stylesheet" href="simple.css" type="text/css" media="screen" />
    <script type="text/javascript" src="simple.js"></script></head>



  <body>
    <div id="main-block">
      <h1>OpenAudio</h1>
      <h3>Description</h3>
      <p>
	OpenAudio is the OpenDarwin interface to IOAudioFamily.
	Currently, it consists of a kernel extension enabling
	you to play sound through your speakers. It implements
	a <code>/dev/dsp</code> entry for each available sound card.
      </p>
      <p>
	Please note that OpenAudio is written in kernel-space,
	and could &ndash; theoretically &ndash; cause your
	computer to self-destruct or &ndash; more likely &ndash;
	cause kernel panics.
      </p>
      <h3>Status</h3>
      <p>
	The project is currently in early development, but is
	known to work on OpenDarwin 6.6. Support for OpenAudio
	was implemented for MPlayer.
      </p>
      <h3>News</h3>
      <p><strong>August 14, 2003:</strong> Homepage created. Current status: Broken on systems based on Darwin 7, including OpenDarwin 7.2.1, see bug
	<a href="http://www.opendarwin.org/bugzilla/show_bug.cgi?id=2013">
	  #2013
	</a>.
      </p>
      <h3>Installation</h3>
      <p>
	
      </p>
      <ul>
	<li>
	  To compile OpenAudio, <code>cd</code> into the source
	  directory and type <code>make</code>.
	</li>
	<li>
	  To create the bundles, type <code>make dist</code>.
	</li>
	<li>
	  To install framework, binaries and kernel extension,
	  type <code>make install</code>, as <code>root</code>.
	</li>
	<li>
	  To load or reload the kernel extension, type <code>make -C
	    kext reload</code>, as <code>root</code>.
	</li>
      </ul>
      <h3>Contact</h3>
      <ul>
	<li>
	  The
	  <a href="http://www.opendarwin.org/mailman/listinfo/openaudio">
	    OpenAudio
	  </a>
	  mailing list.
	</li>
	<li>
	  The
	  <a href="http://www.opendarwin.org/bugzilla/buglist.cgi?product=OpenAudio&amp;order=Last+Changed">
	    Bugzilla
	  </a> bug list.
	</li>
	<li>
	  <a href="irc://irc.freenode.net:6667/#opendarwin">
	    #opendarwin
	  </a> on
	  <a href="http://www.freenode.net/">FreeNode</a>.</li>
      </ul>
      <h3>Developers</h3>
      <ul>
	<li>
	  <a class="mail" href="javascript:send_email('opendarwin.org', 'danchr')">
	    Dan Christiansen
	  </a> - founder and core developer
	</li>
      </ul>
      <p class="footer">
	&copy; Dan Villiom Podlaski Christiansen, 
	<?php echo date("F d Y", getlastmod());?><br />
	This page is valid 
	<a title="HTML Validator" href="http://validator.w3.org/check/referer">
	  XHTML 1.1</a>
	&amp;
	<a title="CSS Validator" href="http://jigsaw.w3.org/css-validator/check/referer">CSS 2.0</a>.
      </p>
    </div>
  </body>
</html>
