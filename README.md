# Royal RASP PHP Extension (Coming Soon)

The royal-php-rasp extension enhances security by monitoring and logging potentially dangerous PHP function calls.

## Compatibility

While we specifically mention PHP-FPM and Nginx due to their widespread use and performance benefits, the extension is compatible with other common setups, including:

- Apache with mod_php: A traditional and widely used setup where PHP runs as a module within the Apache HTTP server.
- CGI/FastCGI: A versatile and standard way to interface external applications with web servers, allowing PHP to be used on a variety of server types.
- Command Line Interface (CLI): For running PHP scripts from the command line, useful for cron jobs and maintenance scripts.

Regardless of your PHP environment, royal-php-rasp provides an additional layer of security by monitoring and logging function calls that could potentially be exploited.

## Prerequisites

- PHP 7.x or 8.x with development tools and headers (`php-dev` or `php-devel`).
- Nginx with PHP-FPM or Apache with mod_php.
- Build tools (make, gcc).
- `phpize` (should come with PHP development tools).

## Features

- Intercept, block, and log malicious PHP functions.
- Written for PHP 8.3, should work on all versions?
- Robust command black-list by checking args.
- File caller is hashed within each log entry.
- Front end log viewer

## Building and Compiling

1. Unzip the source code to a directory, e.g., `/path/to/my_rasp`.

2. Navigate into the directory:
    ```bash
    cd /path/to/my_rasp
    ```

3. Prepare for building:
    ```bash
    phpize
    ```

4. Configure the build environment:
    ```bash
    ./configure
    ```

5. Compile the extension:
    ```bash
    make
    ```

6. Install the compiled extension into PHP's modules directory:
    ```bash
    sudo make install
    ```

## Configuration

### PHP-FPM (Commonly used with Nginx)

1. Locate your PHP-FPM's active `php.ini` file. You can find this by creating a PHP info file or running `php --ini`.

2. Add the following line to `php.ini`:
    ```ini
    extension=my_rasp.so
    ```

3. Restart PHP-FPM to apply changes:
    ```bash
    sudo systemctl restart php-fpm
    ```

4. Update your Nginx server configuration to use PHP-FPM for processing PHP files, if not already set.

### Apache with mod_php

1. Locate Apache's `php.ini` file, similar to the PHP-FPM setup above.

2. Add the `extension=my_rasp.so` line to this `php.ini` file.

3. Restart Apache to apply the changes:
    ```bash
    sudo systemctl restart apache2
    ```

## Verification

To verify the extension is active:

1. Create a `phpinfo.php` file containing `<?php phpinfo(); ?>`.
2. View this page in your web browser. Search the output for "my_rasp" to confirm the extension is loaded.
3. Alternatively, run `php -m | grep my_rasp` from the command line. If the extension is correctly enabled, this command should output `my_rasp`.

## Usage

The My_RASP extension will automatically start logging potentially dangerous function calls according to its predefined rules. By default logs can be found at `/var/www/logfile.log` (specified on line 65 of logger.c).

## Log Viewer

The My_RASP extension comes with a PHP log viewer inside the `/path/to/frontend-log-viewer/viewer.php` by default this PHP script reads the default log file, `/var/www/logfile.log` and prints tables to represent the logs to the HTML document, place this file on your webserver, It is recommended to add some protection as a URI parameter password. Inside the logger.c file changes can be added to insert log entries into a sql database.
However, it is recommended not to connect directly to an SQL database in the raw php extension, instead it's prefered to have a seperate script or program take the log files and the log entries into the database.

## Future updates & considerations

Things to add in the future:

- UI (User Interface) (admin php script) that will allow you to change things like logging method.
- It will allow you to add or remove (enable or disable) new methods to override without modifying the source code.
- Database integration/Remote Database. The admin panel (mentioned above) will have a button to fetch logs, this button will automatically take the logs insert into the database and remove the log files. Then your logs can be viewed in seperate web servers if desired.


## Troubleshooting

Ensure that:
- PHP and your web server (Nginx or Apache) are restarted after installation.<br>
- `my_rasp.so` is correctly placed in PHP's extension directory (check `phpinfo()` for the path).<br>
- `extension=my_rasp.so` is correctly added to your `php.ini` file.
