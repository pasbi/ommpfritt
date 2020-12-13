#!/bin/sh

# Define send message function. parse_mode can be changed to
# HTML, depending on how you want to format your message:
send_msg () {
    curl -s -X POST "${BOT_URL}" -d chat_id=@omm_dev \
        -d text="$1" -d disable_web_page_preview=True \
        -d parse_mode=Markdown
}

BOT_URL="https://api.telegram.org/bot${TELEGRAM_TOKEN}/sendMessage"

if [ "$TRAVIS_TEST_RESULT" -ne 0 ]; then
    # Send message to the bot with some pertinent details about the job
    # Note that for Markdown, you need to escape any backtick (inline-code)
    # characters, since they're reserved in bash
    send_msg "
❗️Build *FAILED*: ${TRAVIS_JOB_NAME}/${TRAVIS_BRANCH})
[Job Log here](${TRAVIS_JOB_WEB_URL})
"
else
    echo "Build succeeded. Not sending telegram notification to avoid clutter."
fi




