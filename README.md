Robin Hood
======================
A free HTTP/1.1 web server for BeOS. It is an original work designed from the ground up for BeOS; it contains no ported code.

Features:

  - Open source code, covered by the GPL.
  - Compatible with both BeOS R5 networking and BONE.
  - HTTP/1.1 compliant
      - HTTP/1.1 Persistant Connections.
      - Supports bytes-ranges.
  - All requests are handled by dynamically loaded add-on modules. Modules are invoked by MIME type.
      - File Handler - File transfer.
      - CGI Handler - Invokes CGI scripts.
      - PHP Handler - Invokes PHP scripts.
      - SSI Handler - Process Server Side Includes.
      - Directory Handler - Displays directory listings with Tracker icons.
      - Redirect Handler - Redirection of resources.
      - 404 Handler - Custom "404 File Not Found" generator.
  - CGI support - As specified in the CGI 1.1 Internet Draft 01.
      - Supports Parsed Header Output as well as Non-Parsed Header Output.
      - Works with shell scripts, Perl, and compiled CGIs.
      - Full CGI environment variable support.
  - PHP support (tested with PHP 4.0.4pl1)
  - SSI support
      - Supports: config, include, echo, fsize, flastmod, and exec.
      - Full SSI environment variable support.
      - Recursive SSI support.
  - Supports If-Modified-Since and If-Unmodified headers.
  - Can automatically send gzip encoded files when the request permits gzip encoding.
  - Basic Authenticaton support
      - User defined Realms.
      - Uses file permissions to control access.
  - Virtual Hosts support.
  - Virtual Resources.
  - Multiport capacity.
  - Separation of user interface and server.

## Requirements

* libHTTP

## Compiling

libHTTP is a requirement to build. Instead of installing libHTTP, you can also specify the location
of the libHTTP headers + libraries at build time.

Example:
```LIBHTTP_H=/Data/libHTTP/headers LIBHTTP_L=/Data/libHTTP/objects.x86_64-cc8-debug make```
