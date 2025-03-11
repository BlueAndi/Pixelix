# PIXELIX <!-- omit in toc -->

![PIXELIX](../images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

## Configurations for the GrabViaMqttPlugin and GrabViaRestPlugin <!-- omit in toc -->

- [General](#general)
- [GrabViaMqttPlugin examples](#grabviamqttplugin-examples)
  - [Grodans Paradis AB outdoor temperature](#grodans-paradis-ab-outdoor-temperature)
- [GrabViaRestPlugin examples](#grabviarestplugin-examples)
  - [Bitcoin price in US$](#bitcoin-price-in-us)
  - [Github repository stargazers](#github-repository-stargazers)
  - [NINA - Warnings](#nina---warnings)
  - [Shelly PlugS - Power consumption](#shelly-plugs---power-consumption)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)
- [Contribution](#contribution)

## General

Please share your configuration for others by pull request or you can open an issue with your example. It will be added here to the list.

## GrabViaMqttPlugin examples

### Grodans Paradis AB outdoor temperature

The author and maintainer of [VSCP](https://www.vscp.org) has an public [MQTT broker](https://github.com/grodansparadis/vscp/wiki/VSCP-Demo-server) and provides some data from his locally installed sensors. This shows how to retrieve e.g. the outdoor temperature at Ake's south house side.

```json
{
    "measurement": {
        "sensorindex": 1,
        "subzone": 0,
        "unit": 1,
        "value": -1.51,
        "zone": 0
    },
    "vscpClass": 1040,
    "vscpData": [1, 0, 0, 1, 45, 49, 46, 53, 49, 48, 48, 48, 48],
    "vscpDateTime": "2025-01-29T23:23:31Z",
    "vscpGuid": "25:00:00:00:00:00:00:00:00:00:00:00:04:01:00:02",
    "vscpHead": 96,
    "vscpNote": "",
    "vscpObId": 0,
    "vscpTimeStamp": 0,
    "vscpType": 6
}
```

The filter is used to identify the value of interest.

```json
    "filter": {
    "measurement": {
      "value": true
    }
  }
```

The bumblebee icon is chosen.

```json
{
    "iconFileId": 0
}
```

The format specifies how the rate will be shown.

```json
{
    "format": "{hc}%0.1f°C"
}
```

No additional manipulation of the value shall take place.

```json
{
    "multiplier": 1,
    "offset": 0
}
```

Example result: ```-1.5°C```

Find the complete plugin configuration in [grodansparadisOutdoorTemperature.json](./rest/grodansparadisOutdoorTemperature.json).

## GrabViaRestPlugin examples

### Bitcoin price in US$

The bitcoin price is get from https://api.coinbase.com/v2/prices/BTC-USD/buy which returns a JSON object. 

```json
{
    "data": {
        "amount": "96145.315",
        "base": "BTC",
        "currency": "USD"
    }
}
```

The filter is used to identify the value of interest.

```json
    "filter": {
        "data": {
            "amount": true
        }
    }
```

No icon is chosen to use the whole display just for the text.

```json
{
    "iconFileId": 255
}
```

The format specifies how the rate will be shown.

```json
{
    "format": "%s $/BTC"
}
```

No additional manipulation of the value shall take place.

```json
{
    "multiplier": 1,
    "offset": 0
}
```

Example result: ```96145.315 $/BTC```

Find the complete plugin configuration in [bitcoin.json](./rest/bitcoin.json).

### Github repository stargazers

The github repositiry stargazers is get for this repository from https://api.github.com/repos/BlueAndi/Pixelix which returns a JSON object. 

```json
{
  "id": 189284754,
  "node_id": "MDEwOlJlcG9zaXRvcnkxODkyODQ3NTQ=",
  "name": "Pixelix",
  "full_name": "BlueAndi/Pixelix",
  "private": false,
  "owner": {
    "login": "BlueAndi",
    "id": 9699465,
    "node_id": "MDQ6VXNlcjk2OTk0NjU=",
    "avatar_url": "https://avatars.githubusercontent.com/u/9699465?v=4",
    "gravatar_id": "",
    "url": "https://api.github.com/users/BlueAndi",
    "html_url": "https://github.com/BlueAndi",
    "followers_url": "https://api.github.com/users/BlueAndi/followers",
    "following_url": "https://api.github.com/users/BlueAndi/following{/other_user}",
    "gists_url": "https://api.github.com/users/BlueAndi/gists{/gist_id}",
    "starred_url": "https://api.github.com/users/BlueAndi/starred{/owner}{/repo}",
    "subscriptions_url": "https://api.github.com/users/BlueAndi/subscriptions",
    "organizations_url": "https://api.github.com/users/BlueAndi/orgs",
    "repos_url": "https://api.github.com/users/BlueAndi/repos",
    "events_url": "https://api.github.com/users/BlueAndi/events{/privacy}",
    "received_events_url": "https://api.github.com/users/BlueAndi/received_events",
    "type": "User",
    "user_view_type": "public",
    "site_admin": false
  },
  "html_url": "https://github.com/BlueAndi/Pixelix",
  "description": "Full RGB LED matrix, based on an ESP32 and WS2812B LEDs.",
  "fork": false,
  "url": "https://api.github.com/repos/BlueAndi/Pixelix",
  "forks_url": "https://api.github.com/repos/BlueAndi/Pixelix/forks",
  "keys_url": "https://api.github.com/repos/BlueAndi/Pixelix/keys{/key_id}",
  "collaborators_url": "https://api.github.com/repos/BlueAndi/Pixelix/collaborators{/collaborator}",
  "teams_url": "https://api.github.com/repos/BlueAndi/Pixelix/teams",
  "hooks_url": "https://api.github.com/repos/BlueAndi/Pixelix/hooks",
  "issue_events_url": "https://api.github.com/repos/BlueAndi/Pixelix/issues/events{/number}",
  "events_url": "https://api.github.com/repos/BlueAndi/Pixelix/events",
  "assignees_url": "https://api.github.com/repos/BlueAndi/Pixelix/assignees{/user}",
  "branches_url": "https://api.github.com/repos/BlueAndi/Pixelix/branches{/branch}",
  "tags_url": "https://api.github.com/repos/BlueAndi/Pixelix/tags",
  "blobs_url": "https://api.github.com/repos/BlueAndi/Pixelix/git/blobs{/sha}",
  "git_tags_url": "https://api.github.com/repos/BlueAndi/Pixelix/git/tags{/sha}",
  "git_refs_url": "https://api.github.com/repos/BlueAndi/Pixelix/git/refs{/sha}",
  "trees_url": "https://api.github.com/repos/BlueAndi/Pixelix/git/trees{/sha}",
  "statuses_url": "https://api.github.com/repos/BlueAndi/Pixelix/statuses/{sha}",
  "languages_url": "https://api.github.com/repos/BlueAndi/Pixelix/languages",
  "stargazers_url": "https://api.github.com/repos/BlueAndi/Pixelix/stargazers",
  "contributors_url": "https://api.github.com/repos/BlueAndi/Pixelix/contributors",
  "subscribers_url": "https://api.github.com/repos/BlueAndi/Pixelix/subscribers",
  "subscription_url": "https://api.github.com/repos/BlueAndi/Pixelix/subscription",
  "commits_url": "https://api.github.com/repos/BlueAndi/Pixelix/commits{/sha}",
  "git_commits_url": "https://api.github.com/repos/BlueAndi/Pixelix/git/commits{/sha}",
  "comments_url": "https://api.github.com/repos/BlueAndi/Pixelix/comments{/number}",
  "issue_comment_url": "https://api.github.com/repos/BlueAndi/Pixelix/issues/comments{/number}",
  "contents_url": "https://api.github.com/repos/BlueAndi/Pixelix/contents/{+path}",
  "compare_url": "https://api.github.com/repos/BlueAndi/Pixelix/compare/{base}...{head}",
  "merges_url": "https://api.github.com/repos/BlueAndi/Pixelix/merges",
  "archive_url": "https://api.github.com/repos/BlueAndi/Pixelix/{archive_format}{/ref}",
  "downloads_url": "https://api.github.com/repos/BlueAndi/Pixelix/downloads",
  "issues_url": "https://api.github.com/repos/BlueAndi/Pixelix/issues{/number}",
  "pulls_url": "https://api.github.com/repos/BlueAndi/Pixelix/pulls{/number}",
  "milestones_url": "https://api.github.com/repos/BlueAndi/Pixelix/milestones{/number}",
  "notifications_url": "https://api.github.com/repos/BlueAndi/Pixelix/notifications{?since,all,participating}",
  "labels_url": "https://api.github.com/repos/BlueAndi/Pixelix/labels{/name}",
  "releases_url": "https://api.github.com/repos/BlueAndi/Pixelix/releases{/id}",
  "deployments_url": "https://api.github.com/repos/BlueAndi/Pixelix/deployments",
  "created_at": "2019-05-29T19:18:13Z",
  "updated_at": "2025-01-27T07:56:00Z",
  "pushed_at": "2025-01-29T21:39:52Z",
  "git_url": "git://github.com/BlueAndi/Pixelix.git",
  "ssh_url": "git@github.com:BlueAndi/Pixelix.git",
  "clone_url": "https://github.com/BlueAndi/Pixelix.git",
  "svn_url": "https://github.com/BlueAndi/Pixelix",
  "homepage": "",
  "size": 53828,
  "stargazers_count": 321,
  "watchers_count": 321,
  "language": "C++",
  "has_issues": true,
  "has_projects": true,
  "has_downloads": true,
  "has_wiki": true,
  "has_pages": false,
  "has_discussions": false,
  "forks_count": 62,
  "mirror_url": null,
  "archived": false,
  "disabled": false,
  "open_issues_count": 5,
  "license": {
    "key": "mit",
    "name": "MIT License",
    "spdx_id": "MIT",
    "url": "https://api.github.com/licenses/mit",
    "node_id": "MDc6TGljZW5zZTEz"
  },
  "allow_forking": true,
  "is_template": false,
  "web_commit_signoff_required": false,
  "topics": [
    "cpp11",
    "display",
    "esp32",
    "esp32-arduino",
    "esp32-s3",
    "pixelix",
    "platformio",
    "rgb-leds",
    "t-display-s3",
    "tc001",
    "ttgo-t-display",
    "ulanzi",
    "ws2812b"
  ],
  "visibility": "public",
  "forks": 62,
  "open_issues": 5,
  "watchers": 321,
  "default_branch": "master",
  "temp_clone_token": null,
  "network_count": 62,
  "subscribers_count": 19
}
```

The filter is used to identify the value of interest.

```json
    "filter": {
        "stargazers_count": true
    }
```

The github icon is chosen and will appear on the left side.

```json
{
    "iconFileId": 2
}
```

The format specifies how the number of stars will be shown.

```json
{
    "format":  "%.0f stars"
}
```

No additional manipulation of the value shall take place.

```json
{
    "multiplier": 1,
    "offset": 0
}
```

Example result: ```321 stars```

Find the complete plugin configuration in [githubStargazers.json](./rest/githubStargazers.json).

### NINA - Warnings

Show the warnings from the Bundesamt für [Bevölkerungsschutz und Katastrophenhilfe](https://www.bbk.bund.de/) in Germany. They provide a [REST API](https://nina.api.bund.dev/) to retrieve several kind of informations.

Get the official regional key of your area here: https://www.xrepository.de/api/xrepository/urn:de:bund:destatis:bevoelkerungsstatistik:schluessel:rs_2021-07-31/download/Regionalschl_ssel_2021-07-31.json

Replace the last 7 digits with "0000000" because the data is only provided at the district level.

Build the URL like ```https://nina.api.proxy.bund.dev/api31/dashboard/<district-key>.json```.

Example:

* Region key for Illertissen: 097750129129
* District key: 097750000000
* URL: ```https://nina.api.proxy.bund.dev/api31/dashboard/097750000000.json```

The URL will provide a JSON object:

```json
[
    {
        "id": "lhp.LHP.BY.16100_774",
        "payload": {
            "version": 5,
            "type": "ALERT",
            "id": "lhp.LHP.BY.16100_774",
            "hash": "e873861e30b4019eb7b14682f92152fdc73a37482e62d3d05c4d767412c6c339",
            "data": {
                "headline": "Entwarnung",
                "provider": "LHP",
                "severity": "Unknown",
                "urgency": "Past",
                "msgType": "Cancel",
                "area": {
                    "type": "GRID",
                    "data": "400592+2,401194+3,401204+3,401807+3,401818+2,402420+4,402430+3,403031+20,403642+24,404254+25,404866+26,405479+26,406095+23,406707+24,407320+23,407933+24,408546+24,409159+24,409773+23,410387+21,411001+20,411616+18,412230+17,412844+15,413457+20,414070+21,414684+20,415297+20,415910+19,416523+19,417135+20,417748+19,418361+19,418974+19,419588+13,419604+3,420201+11,420217+2,420814+9,421427+8,422043+3,422657+2"
                },
                "valid": true
            }
        },
        "i18nTitle": {
            "de": "Entwarnung"
        },
        "sent": "2025-01-29T08:36:01+01:00",
        "onset": "2025-01-29T08:00:00+01:00",
        "expires": "2025-01-30T08:00:00+01:00",
        "effective": "2025-01-29T08:34:00+01:00"
    }
]
```

The filter is used to identify the value of interest.

```json
    "filter": [{
        "i18nTitle": {
            "de": true
        }
    }]
```

Be aware that the i18nTitle is not always for other languages available.

No icon is chosen to use the whole display just for the text.

```json
{
    "iconFileId": 255
}
```

The format specifies how the warning will be shown. Note, there may be several warnings available in the response. They will be automatically concatenated with "::" as delimiter.

```json
{
    "format": "%s"
}
```

No additional manipulation of the string is possible, so we don't care about the multiplier and the offset.

```json
{
    "multiplier": 1,
    "offset": 0
}
```

Example result: ```Entwarnung```

Find the complete plugin configuration in [nina.json](./rest/nina.json).

### Shelly PlugS - Power consumption

The Shelly PlugS provide the power consumption via [REST API](https://shelly-api-docs.shelly.cloud/gen1/#shelly-plug-plugs-meter-0), e. g. http://192.168.1.123/meter/0/.

```json
{
    "power": 0,
    "overpower": 23.78,
    "is_valid": true,
    "timestamp": 0,
    "counters": [1,2,3],
    "total": 4
}
```

The filter is used to identify the value of interest.

```json
    "filter": {
        "power": true
    }
```

The lightning icon is chosen.

```json
{
    "iconFileId": 5
}
```

The format specifies how the power consumption will be shown.

```json
{
    "format": "{hc}%0.1f W"
}
```

No additional manipulation of the value shall take place.

```json
{
    "multiplier": 1,
    "offset": 0
}
```

Example result: ```0.0 W```

Find the complete plugin configuration in [shellyPlugSPower.json](./rest/shellyPlugSPower.json).

## Issues, Ideas And Bugs

If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/Pixelix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

## License

The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

## Contribution

Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
