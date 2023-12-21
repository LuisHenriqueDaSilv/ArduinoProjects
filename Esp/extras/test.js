fetch("https://cdn-icons-png.flaticon.com/512/5683/5683514.png")
  .then(response => response.blob())
  .then(blob => {
    const url = URL.createObjectURL(blob);
    console.log(url)
});