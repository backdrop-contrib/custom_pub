(function ($) {
  "use strict";
  Drupal.behaviors.custom_pub_admin = {
    attach: function () {
      $("td.custom_pub-option-edit-cell").html('<a href="#">Edit</a>').css("text-align", "right");
      $("tr.custom_pub-form-edit").hide();
      $("td.custom_pub-option-edit-cell > a").bind('click', function () {
        var $this = $(this);
        var opt = [];
        var txt = $this.text();
        opt["Edit"] = "Close";
        opt["Close"] = "Edit";
        $this.parents('tr.custom_pub-option-row').next("tr.custom_pub-form-edit").toggle();
        $this.text(opt[txt]);
        return false;
      });
      $("th.close-custom-pub-table").html('<a>X</a>')
      .css('text-align', 'right');
      $("th.close-custom-pub-table > a")
      .css('cursor', 'pointer')
      .live('click', function () {
        var $this = $(this);
        var opt = [];
        var $row = $this.parents("tr.custom_pub-form-edit");
        var $link = $row.prev('tr.custom_pub-option-row').find("td.custom_pub-option-edit-cell > a");
        var txt = $link.text();
        opt["Edit"] = "Close";
        opt["Close"] = "Edit";
        $row.toggle();
        $link.text(opt[txt]);
        return false;
      });
    }
  };
})(jQuery);
