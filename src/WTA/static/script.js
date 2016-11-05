$(function() {
	$('button').click(function() {
		var query = $('#query').val();

		$.ajax({
			url  : '/response',
			data : query,
			type : 'POST',
			success : function (response) {
				alert('Success!');
				console.log(response);
			},
			error : function(error) {
				alert('Errror!');
				console.log(error);
			}
 		});
	})
});