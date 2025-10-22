module.exports = [
    {
      "type": "heading",
      "defaultValue": "Witching Hour Configuration"
    },
    {
      "type": "section",
      "items": [
        {
          "type": "heading",
          "defaultValue": "Time and Date"
        },
        {
          "type": "toggle",
          "messageKey": "AmericanDate",
          "label": "Use American date format",
          "defaultValue": true,
          "description": "Set true for 'Oct 31' and false for '31 Oct'"
        }
      ]
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Weather"
            },
            {
                  "type": "slider",
                  "label": "Weather update rate (in minutes):",
                  "messageKey": "WeatherCheckRate",
                  "defaultValue": "30",
                  "min": 15,
                  "max": 120,
                  "step": 15
              },
              {
                  "type": "toggle",
                  "messageKey": "UseCurrentLocation",
                  "label": "Use current location",
                  "defaultValue": true
              },
              {
                  "type": "input",
                  "messageKey": "Latitude",
                  "defaultValue": "",
                  "label": "Manual Latitude",
                  "attributes": {
                      "placeholder": "eg: 42.36",
                      "type": "number"
                  }
              },
              {
                  "type": "input",
                  "messageKey": "Longitude",
                  "defaultValue": "",
                  "label": "Manual Longitude",
                  "attributes": {
                      "placeholder": "eg: -71.10",
                      "type": "number"
                  }
              },
              {
                  "type": "toggle",
                  "messageKey": "TemperatureMetric",
                  "label": "Use metric units for temperature",
                  "defaultValue": false,
                  "description": "Set true for Celsius and false for Fahrenheit"
              },
              {
                "type": "input",
                "messageKey": "Temperature4",
                "defaultValue": "90",
                "label": "Above what temperature is \"hot\" to you?<br><img alt=\"[Hot Witch Preview]\" src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACUAAAAvCAYAAABkBQNlAAAAAXNSR0IB2cksfwAAAARnQU1BAACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAAAAZiS0dEAP8A/wD/oL2nkwAAAAlwSFlzAAAuIwAALiMBeKU/dgAAAAd0SU1FB+kKFgEQAIldiG4AAAF3SURBVFjD7VjRDoQwCKOL///L3MstmTgYDDQ+uORy0Z3aa7vCJPrGN77xjXcPJK/nG+65fQMmImK+YgKQBoZiMMTMaWCRC3kGpgPqc08x5WKnUkJ4wYxszI4rvXV4mFkBqDZ703zTpTHkOf1GzmVG00wrmbLm+3nhMa4Axf1hIxNyZWky9vPim7OgMMkamsk5ArHk2gV2SKYsMBGzC+AcMT4cRocVnKOslryRFQmjyGKV5FqqZ6MCa/Z5C0CGqRYuAYEcGldxxPRtfV+kAnFk1Qvs8MqXXYWpRK/sv4w/xKWe2lfRD6wVP/n0ETHjBoaop7SOYeW1IRJYKfZIg9JAOrNq2rN1PPD5dd0lbAbojDUcmdxxbiIsRvEv1ifl4F/dfKFfkYCMOVTnjtZ+sOwQZmzeVftgZVDlzqYip9iS8IkyEyovu9ut9jaWbmEqy1IW1C0slTNVwdKTrctjoC6tyGuNnt0dp9+4iX3ipV+qeKtXSlTmrcsPFZwgXncjE2QAAAAASUVORK5CYII=\" />",
                "attributes": {
                  "placeholder": "90",
                  "type": "number"
                }
              },
              {
                "type": "input",
                "messageKey": "Temperature3",
                "defaultValue": "75",
                "label": "Above what temperature is \"warm\" to you?<br><img alt=\"[Warm Witch Preview]\" src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACUAAAAvCAYAAABkBQNlAAAAAXNSR0IB2cksfwAAAARnQU1BAACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAAAAZiS0dEAP8A/wD/oL2nkwAAAAlwSFlzAAAuIwAALiMBeKU/dgAAAAd0SU1FB+kKFgEQCmmIYXAAAAGfSURBVFjD1VjtboQwDMMV7//K2Y+tW4maNJ/idtLpJODA2I6T9ro+8APHtRT8XxsoIvrDBCDzcrVMrcAeN/kGWQZs/HdP/fpqx1glWyPwEth46lWmLKZPMxaSb4OyVMqS6quuSHdOWfzErkUrU1rlcdBE1ArqFAOSp1rkIy4FrzrNYxlP3RZmTgCqg3RYDC3JszN9RbAOybRSQO7Oz+PMY1QBiubDViZ4ZUkyzuPsl7KgsCnnS8un9ToFKGWMTicwHrMz4OQxPgxGx6nFTFk1eT0VCaXpwtPzLLFhBYYz+xQCkGHKPQ57cmitYo/px/m+SAXiyqoV2B0ZWSJV2D2ju+JAeCH6hCUWPMBG8ZMfXxYzZmDwekoZ6KzzlbQaQhqUBNKYVduZbeKBza/nKSEYoDvWcGdy53TeIC1+mvVDOdcSi9MvSHAp51CdO9L4QXxC2LHZ1fugZVDlyqYipywbHu9vmkW9VAWqnKUWprIsZUERX469Nbqoy6wNSLwm3yrZaS3YDQpcukiTbmVKqMIQugp38oZK2e3Frg9ldl2+AJ7yQ3KkgsRkAAAAAElFTkSuQmCC\" />",
                "attributes": {
                  "placeholder": "75",
                  "type": "number"
                }
              },
              {
                "type": "input",
                "messageKey": "Temperature2",
                "defaultValue": "60",
                "label": "Above what temperature is \"chilly\" to you? <img alt=\"[Chilly Witch Preview]\" src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACUAAAAvCAYAAABkBQNlAAAAAXNSR0IB2cksfwAAAARnQU1BAACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAAAAZiS0dEAP8A/wD/oL2nkwAAAAlwSFlzAAAuIwAALiMBeKU/dgAAAAd0SU1FB+kKFgEQFJOHXBMAAAGcSURBVFjD1VnZjsMwCCwo///L7MMqrcOCzTFOupGqSmmO8TCMgb5eX3hQ4lop3rcNlIh8MBFRZ3FYpkZgl4f8goQB4/+uqbeuLMaQbHFhEWRo6lGmIqJvM1YKn4ESGkpI9qEzMu1TET2pa2krU7PM06BFZCuolQ14mtoSPtGh0Fk301hHU0eEmRUAtJFyRNBeeCzRI4yVPdF6Bmn9fp5XGhMEKDlfNjKhM8sL43lefQtC6GKldjDTYO5OGpAHxrOFHcKngNApu8WgQY06oMqehwDGXs00A7S7nuLiitPXZrKRdwGa2IbAmcpmoWMZ8lg349VgK2B3tViUAcbgN18+ymbCwGgdgXkJE3X8wacuO4eFZQlqpZNinWXWbCceiul1XSUUXd3qIemoPBix7QwJ8GfElGqxNP2VyiGyB2ZbLLqjSc1YAkXb+O5xNO4VoLaeG5pFa6puYZQSfBTU0V39avBRGXa0NaV9S5thpQZjBFPGpGVkhlSJfY/QNVPdCpV7yUTmbKFrFTCmnNkDpG1v2YJuXNH/RCCAlmP4A26KImoGfGkbAAAAAElFTkSuQmCC\" />",
                "attributes": {
                  "placeholder": "60",
                  "type": "number"
                }
              },
              {
                "type": "input",
                "messageKey": "Temperature1",
                "defaultValue": "40",
                "label": "Above what temperature is \"cold\" to you? <img alt=\"[Cold Witch Preview]\" src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACUAAAAvCAYAAABkBQNlAAAAAXNSR0IB2cksfwAAAARnQU1BAACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAAAAZiS0dEAP8A/wD/oL2nkwAAAAlwSFlzAAAuIwAALiMBeKU/dgAAAAd0SU1FB+kKFgEQGwM4QYIAAAGESURBVFjD1VhbkoMwDEMZ7n9l92P7oFmbyC/aMsMPQ4iiKLLFtn3hBce7EhzXBkpEXpgAZBZXy9QR2NtH/kCWARu/rqmnrjTGKtkagUVA0dRHmWJEn2YstH0KytKtLDl91SfS7VOMnqZ30crU2cmbQYtIK6iVDViaatk+mVa8MSLv0JR4JtbYyrJ0HCirycnTlhb5Y5BkwHTYAjwepGlroakwKDk5Pax7b3PpyWwjLIFrZskIvqLsgHDutO6yoI6sufRWCWxYPdMZoO5+akT9yPvufXFSDirLEAtsJD4eWggDrDXNWD3YCthVEQseYKN45rd7shkaGNY7oJcPxskNnxKjR6OL5bL8BMOEaPXygQecXsVkKunqWobEHvlwYf8FAP9+Mbki1kx/pFtlaqA3YuGKkOqxBLAxPnvtibFtvf2lP83YnirbGLkEz4Las6tf/fiIBAhUaspKNt759kKtnD0LRaywnjRWshErc/pQ1dSVW4IlcOO5dG9fxNV7sxkBNGz3N+ej9V1DFDgxAAAAAElFTkSuQmCC\" />",
                "attributes": {
                  "placeholder": "40",
                  "type": "number"
                }
              },
              { 
                "type": "text",
                "defaultValue": "Below \"cold\" will be freezing <img alt=\"[Freezing Witch Preview]\" src=\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACUAAAAvCAYAAABkBQNlAAAAAXNSR0IB2cksfwAAAARnQU1BAACxjwv8YQUAAAAgY0hSTQAAeiYAAICEAAD6AAAAgOgAAHUwAADqYAAAOpgAABdwnLpRPAAAAAZiS0dEAP8A/wD/oL2nkwAAAAlwSFlzAAAuIwAALiMBeKU/dgAAAAd0SU1FB+kKFgEQIcU0mDAAAAGuSURBVFjDxVhJbsMwEAsF/f/L00PsQnFmX1oDPsSILJpDcSi9Xr6LrvtPLngAEb3xAMiMD187OuAGeIGkCWAIlO8D0AGsnbFVBX8Bpf9g6ktfk4zt8ld9ir9FY2GmNOF3MdZWvk5g6AbUAQxRQIyBil6WBYaoniSveoImolFQlnFKYh8pHz2+2Gw7U5qiyMQcW1WWzoFkTe410arI70FUATNhC0iYYkRTqVKyTEWEza3A0yIyjMHqaSfAcyVqH8IBjACDw7lFNq2SZxmDkgRMvVmsZROE9Qey2srEilzeF913IQCeH0PZ5PnLkpcVTezns0v4YkrdGfqlGCOB555rwEoZ3WLwtoLoyl1TeuLKer/nIQsKM8VpwytszeUZG0LKEqRSVCIOlypMUFb7SWwmzO1ZunyFuKP2WwC0o6vNWjmBeAMAfMeI7vcquhLazHMAymcJ1RjNEbN8+oT2u93fQodmWqTxthxPSkCtarGydeUpN4OdpzG7S+SSdjL7wNbySTub6HwtlsCxVN33lfSkJczsvm9VCPLqK3qsvSZKpzyn6fKFJmqcrwQ03RB/ABFYRngp3h5OAAAAAElFTkSuQmCC\" />"
              }
        ]
    },
    {
      "type": "section",
      "items": [
        {
          "type": "heading",
          "defaultValue": "Vibration"
        },
        {
          "type": "toggle",
          "messageKey": "VibrateOnDisc",
          "label": "Vibrate on bluetooth disconnect",
          "defaultValue": true
        }
      ]
    },
    {
        "type": "submit",
        "defaultValue": "Submit"
    }
  ];
  