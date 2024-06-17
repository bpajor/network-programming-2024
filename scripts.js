function showMessage() {
    document.getElementById('message').innerText = 'Hello, you clicked the button!';
}

function sendPost(event) {
    event.preventDefault();
    const formData = new FormData(document.getElementById('postForm'));
    fetch('/post', {
        method: 'POST',
        body: new URLSearchParams(formData)
    }).then(response => {
        if (response.ok) {
            window.location.href = "/confirm";
        }
    }).catch(error => console.error('Error:', error));
}