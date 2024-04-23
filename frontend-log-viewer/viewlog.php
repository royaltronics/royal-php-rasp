<?php
// Path to the My_RASP log file
$logfile = '/var/www/logfile.log';
if (file_exists($logfile)) {
    $logEntries = file($logfile, FILE_SKIP_EMPTY_LINES);
} else {
    echo "There are no logs yet.";
    $logEntries = [];  // Initialize as empty array to avoid further errors
}
//echo 'Remote IP: ' . $_SERVER['REMOTE_ADDR'];

?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Log Viewer</title>
    <style>
        /* Basic styling for readability and layout */
        body { font-family: Arial, sans-serif; margin: 0; padding: 0; }
        table { width: 100%; border-collapse: collapse; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #f2f2f2; } /* Header background */
        tr:nth-child(even) { background-color: #f9f9f9; } /* Zebra-striping for rows */
    </style>
</head>
<body>
    <h1>Log Entries</h1>
    <table>
        <thead>
            <tr>
                <th>Timestamp</th>
                <th>Type</th>
                <th>Details</th>
                <th>Caller</th>
                <th>Filename</th>
                <th>Line</th>
                <th>File Hash</th>
                <th>From Eval()</th>
                <th>Was Blocked</th>
                <th>File Last Modified</th>
                <th>IP</th>
            </tr>
        </thead>
        <tbody>
            <?php foreach ($logEntries as $entry): ?>
                <?php
                // Attempt to decode the individual JSON log entry into an associative array.
                $log = json_decode($entry, true);
                if ($log): // Check if the JSON log entry is valid and can be decoded.
                ?>
                    <tr>
                        <!-- Safely escape and display the log details to avoid XSS attacks -->
                        <td><?= htmlspecialchars($log['timestamp'] ?? 'N/A') ?></td>
                        <td><?= htmlspecialchars($log['type'] ?? 'N/A') ?></td>
                        <td><?= htmlspecialchars($log['details'] ?? 'N/A') ?></td>
                        <td><?= htmlspecialchars($log['caller'] ?? 'N/A') ?></td>
                        <td><?= htmlspecialchars($log['filename'] ?? 'N/A') ?></td>
                        <td><?= htmlspecialchars($log['line'] ?? 'N/A') ?></td>
                        <td><?= htmlspecialchars($log['file-hash'] ?? 'N/A') ?></td>
                        <td><?= isset($log['is-eval']) ? ($log['is-eval'] ? 'Yes' : 'No') : 'N/A' ?></td>
                        <td><?= isset($log['was-blocked']) ? ($log['was-blocked'] ? 'Yes' : 'No') : 'N/A' ?></td>
                        <td><?= htmlspecialchars($log['modified-time'] ?? 'N/A') ?></td>
                        <td><?= htmlspecialchars($log['ip'] ?? 'N/A') ?></td>
                    </tr>
                <?php endif; ?>
            <?php endforeach; ?>
        </tbody>
    </table>
</body>
</html>
