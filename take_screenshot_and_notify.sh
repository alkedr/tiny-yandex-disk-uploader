URL=$(/home/alkedr/sync/programming/tiny-yandex-disk-uploader/tiny-yandex-disk-uploader)
echo "${URL}" | xclip
notify-send 'Скриншот загружен' "URL: ${URL}\nURL скопирован в буфер обмена" --icon=dialog-information --expire-time=1000
